#include <setjmp.h>
#include <stdio.h>
#include <string.h>

#include "object.h"
#include "api.h"
#include "utils.h"
#include "zio.h"
#include "bytecode.h"

namespace nf::imp {

const Size basic_ci_nr = 8;
const Size basic_stack_nr = 32;
const TValue nil = { .type = Type::NIL, .i = 0 };

static void Thread_stack_init(Thread* self)
{
    self->stack = NF_ALLOC_ARRAY_P(self, TValue, basic_stack_nr);
    self->stack_nr = basic_stack_nr;
    self->top = self->stack;
    self->base = self->stack;
}

static void StrTab_init(Thread* th, StrTab* self)
{
    self->buckets_nr = 0;
    self->buckets = nullptr;

    self->buckets = NF_REALLOC_ARRAY_P(th, self->buckets, Str*, 31);
    self->buckets_nr = 31;
    for (int i = 0; i < 31; i++)
        self->buckets[i] = nullptr;
}

void StrTab_insert(Thread* th, StrTab* self, Str* str)
{
    (void)th;
    Size bucket_index = str->hash % self->buckets_nr;
    // Str* ptr = self->buckets[bucket_index];
    // while (ptr) {
    //     if (ptr->hash == str->hash && ptr->nr == str->nr) {
    //         if (memcmp(ptr->base, str->base, str->nr) == 0) {
    //             return false;
    //         }
    //     }
    //     ptr = (Str*)(ptr->next);
    // }

    str->next = self->buckets[bucket_index];
    self->buckets[bucket_index] = str;
}

Str* StrTab_search(StrTab* self, const char* str, Size len, Hash hash)
{
    Size bucket_index = hash % self->buckets_nr;
    Str* ptr = self->buckets[bucket_index];
    while (ptr) {
        if (ptr->hash == hash && ptr->nr == len) {
            if (memcmp(ptr->base, str, len) == 0) {
                return ptr;
            }
        }
        ptr = (Str*)(ptr->next);
    }

    return nullptr;
}

static void Thread_init_step_one(Thread* self)
{
    TValue_set_nil(&(self->gt));
    TValue_set_nil(Thread_registry(self));
    auto g = Thread_global(self);
    g->root = self;
    StrTab_init(self, &(g->str_tab));

    self->error_jmp = nullptr;

    self->stack = nullptr;
    self->stack_nr = 0;

    self->pc = nullptr;

    self->base = nullptr;
    self->top = nullptr;

    self->func = nullptr;

    self->error_msg[0] = 0;

    self->up_values = nullptr;
    ;
    self->up_values_nr = 0;
    self->up_values_alloc = 0;
    self->closed_uv_nr = 0;

    self->debug = false;

    MBuffer_init(&(self->tmp_buf));
}

static void Thread_init_step_two(Thread* self, void* unused)
{
    (void)unused;
    Thread_stack_init(self);

    auto table = Table_new(self);
    TValue_set_table(Thread_registry(self), table);
}

struct Twin {
    Thread thread;
    GlobalState global;
};

void Thread_close(Thread* self)
{
    NF_FREE(self->stack);
    NF_FREE(self);
}

Thread* Thread_open()
{
    auto twin = NF_ALLOC_T(Twin);
    if (!twin)
        return nullptr;

    auto self = &(twin->thread);
    Thread_global(self) = &(twin->global);
    self->type = Type::Thread;

    Thread_init_step_one(self);

    if (E::OK != Thread_run_protected(self, Thread_init_step_two, nullptr)) {
        Thread_close(self);
        self = nullptr;
    }

    return self;
}

Error Thread_run_protected(Thread* self, ProtectedFunc f, void* ud)
{
    LongJmp recover;
    recover.status = E::OK;
    recover.prev = self->error_jmp;
    recover.msg = nullptr;
    self->error_jmp = &recover;

    if (setjmp(recover.b) >= 0) {
        f(self, ud);
    }

    if (recover.msg) {
        snprintf(self->error_msg, ERROR_MSG_NR - 1, "%s", recover.msg);
    }

    self->error_jmp = recover.prev;
    return recover.status;
}

void Thread_throw(Thread* self, Error err, const char* msg)
{
    if (self->error_jmp) {
        self->error_jmp->status = err;
        self->error_jmp->msg = msg;
        longjmp(self->error_jmp->b, -1);
    } else {
        fprintf(stderr, "throw without protect: %d %s\n", (int)err,
            msg ?: self->error_msg);
        self->status = err;
        exit(EXIT_FAILURE);
    }
}

struct LoadS {
    const char* s;
    size_t size;
};

static const char* LoadS_read(Thread* th, void* ud, size_t* size)
{
    (void)th;
    LoadS* self = (LoadS*)ud;
    if (self->size == 0)
        return nullptr;
    *size = self->size;
    self->size = 0;
    return self->s;
}

Error Thread_load(Thread* self, Reader reader, void* data, const char* name)
{

    ZIO z;
    Error err = E::LOAD;
    if (!name)
        name = "?";
    ZIO_init(self, &z, reader, data);
    err = protected_parser(self, &z, name);
    return err;
}

Error Thread_load(Thread* self, const char* buff, size_t size, const char* name)
{
    LoadS ls;
    ls.s = buff;
    ls.size = size;
    return Thread_load(self, LoadS_read, &ls, name);
}

#define NUMBERIC_BIN_OP(_OP_)                                                  \
    if (first->type == Type::Integer) {                                        \
        if (second->type == Type::Integer) {                                   \
            *stack_slot(self, result_slot) = { .type = Type::Integer,          \
                                               .i = first->i _OP_ second->i }; \
        } else if (second->type == Type::Number) {                             \
            *stack_slot(self, result_slot) = { .type = Type::Number,           \
                                               .n = first->i _OP_ second->n }; \
        }                                                                      \
    } else if (first->type == Type::Number) {                                  \
        if (second->type == Type::Integer) {                                   \
            *stack_slot(self, result_slot) = { .type = Type::Number,           \
                                               .n = first->n _OP_ second->i }; \
        } else if (second->type == Type::Number) {                             \
            *stack_slot(self, result_slot) = { .type = Type::Number,           \
                                               .n = first->n _OP_ second->n }; \
        }                                                                      \
    }

#define NUMBERIC_CMP_OP(_OP_)                                                  \
    if (first->type == Type::Integer) {                                        \
        if (second->type == Type::Integer) {                                   \
            *stack_slot(self, result_slot) = { .type = Type::Bool,             \
                                               .b = first->i _OP_ second->i }; \
        } else if (second->type == Type::Number) {                             \
            *stack_slot(self, result_slot) = { .type = Type::Bool,             \
                                               .b = first->i _OP_ second->n }; \
        }                                                                      \
    } else if (first->type == Type::Number) {                                  \
        if (second->type == Type::Integer) {                                   \
            *stack_slot(self, result_slot) = { .type = Type::Bool,             \
                                               .b = first->n _OP_ second->i }; \
        } else if (second->type == Type::Number) {                             \
            *stack_slot(self, result_slot) = { .type = Type::Bool,             \
                                               .b = first->n _OP_ second->n }; \
        }                                                                      \
    }

static void __Thread_close_uv_to(Thread* self, StackIndex new_top)
{
    for (uint64_t i = self->closed_uv_nr; i < self->up_values_nr; i++) {

        if (self->up_values[i]->abs_stack_index
            >= new_top + self->base - self->stack) {

            auto closed_uv_nr = self->closed_uv_nr;

            if (i != closed_uv_nr) {
                auto tmp = self->up_values[closed_uv_nr];
                self->up_values[closed_uv_nr] = self->up_values[i];
                self->up_values[i] = tmp;
            }

            auto closed_uv = self->up_values[closed_uv_nr];

            closed_uv->closed = true;
            closed_uv->value = *(self->stack + closed_uv->abs_stack_index);
            self->closed_uv_nr++;
        }
    }
}

static void __Thread_close_uv(Thread* self, StackIndex will_pop)
{
    __Thread_close_uv_to(self, self->top - will_pop - self->base);
}

static void __Thread_return(Thread* self,
                            StackIndex called_func,
                            StackIndex desire_retvals_nr,
                            StackIndex retn)
{
    auto return_value_base = self->top - retn;

    self->top = self->base;
    self->top = Thread_pop_index(self) + self->stack;
    self->base = Thread_pop_index(self) + self->stack;
    self->pc = Thread_pop_pc(self);
    self->func = self->func->prev;
    self->top = self->base + normalize_stack_index(self, called_func);

    for (int i = 0; i < retn; i++)
        self->top[i] = return_value_base[i];
    self->retvals_nr = retn;
    self->top += retn;

    if (retn < desire_retvals_nr) {
        for (auto i = retn; i < desire_retvals_nr; i++) {
            Thread_push_nil(self);
        }
    } else
        self->top -= (retn - desire_retvals_nr);

    __Thread_close_uv(self, 0);
}

static void Thread_call(Thread* self,
                        StackIndex func_i,
                        StackIndex desire_retvals_nr);

static int __Thread_run(Thread* self)
{
    while (self->pc) {
        Instruction ins = *(self->pc++);
        if (self->debug) {
            printf("run(%ld, %ld) %s %d %d \n",
                   self->top - self->base,
                   self->top - self->stack,
                   opcode_names[(int)(INS_OP(ins))],
                   (int)(INS_AB(ins)),
                   (int)(INS_CD(ins)));
        }

        switch (INS_OP(ins)) {
            case Opcode::RET_0: {
                return 0;
            }

            case Opcode::RET_TOP: {
                return 1;
            }

            case Opcode::ADD: {
                StackIndex first_slot = INS_AB(ins);
                StackIndex second_slot = INS_CD(ins);
                StackIndex result_slot = INS_EF(ins);

                TValue* first = stack_slot(self, first_slot);
                TValue* second = stack_slot(self, second_slot);
                if (is_numberic(first) && is_numberic(second)) {
                    NUMBERIC_BIN_OP(+);
                }

                else if (is_str(first) && is_str(second)) {
                    auto concat = Str_concat(
                        self, tv2str(first), tv2str(second));
                    *(stack_slot(self, result_slot)) = {
                        .type = Type::String,
                        .obj = concat,
                    };
                } else {
                    Thread_throw(self, E::OP_NUM);
                }
                break;
            }

            case Opcode::SUB: {
                StackIndex result_slot = INS_EF(ins);
                StackIndex first_slot = INS_AB(ins);
                StackIndex second_slot = INS_CD(ins);

                TValue* first = stack_slot(self, first_slot);
                TValue* second = stack_slot(self, second_slot);
                if (is_numberic(first) && is_numberic(second)) {
                    NUMBERIC_BIN_OP(-);
                } else {
                    Thread_throw(self, E::OP_NUM);
                }
                break;
            }

            case Opcode::MUL: {
                StackIndex result_slot = INS_EF(ins);
                StackIndex first_slot = INS_AB(ins);
                StackIndex second_slot = INS_CD(ins);

                TValue* first = stack_slot(self, first_slot);
                TValue* second = stack_slot(self, second_slot);
                if (is_numberic(first) && is_numberic(second)) {
                    NUMBERIC_BIN_OP(*);
                } else {
                    Thread_throw(self, E::OP_NUM);
                }
                break;
            }

            case Opcode::DIV: {
                StackIndex result_slot = INS_EF(ins);
                StackIndex first_slot = INS_AB(ins);
                StackIndex second_slot = INS_CD(ins);

                TValue* first = stack_slot(self, first_slot);
                TValue* second = stack_slot(self, second_slot);
                if (is_numberic(first) && is_numberic(second)) {
                    NUMBERIC_BIN_OP(/);
                } else {
                    Thread_throw(self, E::OP_NUM);
                }
                break;
            }

            case Opcode::CONST: {
                auto const_index = (StackIndex)INS_AB(ins);
                auto* tv = &(self->func->proto->const_arr[const_index]);
                Thread_push(self, tv);
                break;
            }

            case Opcode::LOAD_NIL: {
                TValue tv = { .type = Type::NIL, .n = 0 };
                Thread_push(self, &tv);
                break;
            }

            case Opcode::PUSH: {
                auto slot = (StackIndex)INS_AB(ins);
                Thread_push(self, (stack_slot(self, slot)));
                break;
            }

            case Opcode::SET: {
                auto slot = (StackIndex)INS_AB(ins);
                auto value_slot = (StackIndex)INS_CD(ins);
                *(stack_slot(self, slot)) = *(stack_slot(self, value_slot));
                break;
            }

            case Opcode::NEW_TABLE: {
                auto table = Table_new(self);
                TValue v = { .type = Type::Table, .obj = table };
                Thread_push(self, &v);
                break;
            }

            case Opcode::TABLE_SET: {
                TValue* value = self->top - 1;
                auto table_slot = (StackIndex)INS_AB(ins);
                auto key_slot = (StackIndex)INS_CD(ins);
                auto key = stack_slot(self, key_slot);
                auto table = tv2table(stack_slot(self, table_slot));
                *Table_set(self, table, key) = *value;
                self->top -= 1;
                break;
            }

            case Opcode::TABLE_GET: {
                auto slot = (StackIndex)INS_AB(ins);
                auto table = tv2table(stack_slot(self, slot));

                auto key_slot = (StackIndex)INS_CD(ins);
                auto key = stack_slot(self, key_slot);
                auto value = Table_get(self, table, key);

                Thread_push_nil(self);

                if (value)
                    *(self->top - 1) = *value;
                else
                    (self->top - 1)->type = Type::NIL;
                break;
            }

            case Opcode::POP_TO: {

                auto new_top = (StackIndex)INS_AB(ins);
                self->top = new_top + self->base;

                break;
            }

            case Opcode::NEG: {
                auto v = (StackIndex)INS_AB(ins) + self->base;
                TValue result = *v;
                switch (v->type) {
                    case Type::Integer: {
                        result.i = -v->i;
                        break;
                    }
                    case Type::Number: {
                        result.n = -v->n;
                        break;
                    }
                    default: {
                        Thread_throw(self, E::OP_NUM, "unsupport neg");
                    }
                }
                Thread_push(self, &result);
                break;
            }
            case Opcode::LEN: {
                auto v = (StackIndex)INS_AB(ins) + self->base;
                TValue result = { .type = Type::Integer, .i = 0 };
                switch (v->type) {
                    case Type::String: {
                        result.i = tv2str(v)->nr;
                        break;
                    }
                    default: {
                        Thread_throw(self, E::OP_NUM, "unsupport neg");
                    }
                }
                Thread_push(self, &result);
                break;
            }
            case Opcode::CALL: {
                auto slot = (StackIndex)INS_AB(ins);
                auto desire_retvals_nr = (StackIndex)INS_CD(ins);
                Thread_call(self, slot, desire_retvals_nr);
                break;
            }
            case Opcode::NEW_NF_FUNC: {
                auto proto = tv2proto(self->top - 1);
                auto func = Func_new(self, proto);
                self->top--;
                Thread_push_func(self, func);
                break;
            }
            case Opcode::GET_UP_VALUE: {
                auto uv_index = (StackIndex)INS_AB(ins);
                auto uv = self->func->up_values[uv_index];

                if (uv->closed) {
                    Thread_push(self, &(uv->value));
                } else {
                    auto abs_stack_index
                        = self->func->up_values[uv_index]->abs_stack_index;
                    Thread_push(self, abs_stack_index + self->stack);
                }
                break;
            }

            case Opcode::SET_UP_VALUE: {
                auto uv_index = (StackIndex)INS_AB(ins);
                auto value_slot = (StackIndex)INS_CD(ins);

                auto uv = self->func->up_values[uv_index];

                if (uv->closed) {
                    uv->value = *stack_slot(self, value_slot);
                } else {
                    auto abs_stack_index
                        = self->func->up_values[uv_index]->abs_stack_index;
                    self->stack[abs_stack_index] = *stack_slot(self,
                                                               value_slot);
                }
                break;
            }

            case Opcode::CLOSE_UV_TO: {
                __Thread_close_uv_to(self, (StackIndex)INS_AB(ins));
                break;
            }

            case Opcode::JUMP: {
                InsIndex target = (InsIndex)INS_ABCD(ins);
                self->pc = self->func->proto->ins + target;

                break;
            }

            case Opcode::JUMP_IF_FALSE: {
                auto cond = self->top - 1;
                if (cond->type == Type::NIL
                    || (cond->type == Type::Bool && cond->b == false)) {
                    InsIndex target = (InsIndex)INS_ABCD(ins);
                    self->pc = self->func->proto->ins + target;
                }

                break;
            }

            case Opcode::LESS: {
                StackIndex result_slot = INS_EF(ins);
                StackIndex first_slot = INS_AB(ins);
                StackIndex second_slot = INS_CD(ins);

                TValue* first = stack_slot(self, first_slot);
                TValue* second = stack_slot(self, second_slot);
                if (is_numberic(first) && is_numberic(second)) {
                    NUMBERIC_CMP_OP(<);
                } else if (is_str(first) && is_str(second)) {
                    auto x = Str_cmp(tv2str(first), tv2str(second));
                    *stack_slot(self, result_slot) = {
                        .type = Type::Bool,
                        .b = x == -1,
                    };
                } else {
                    Thread_throw(self, E::OP_NUM);
                }

                break;
            }

            case Opcode::LE: {
                StackIndex first_slot = INS_AB(ins);
                StackIndex result_slot = INS_EF(ins);
                StackIndex second_slot = INS_CD(ins);

                TValue* first = stack_slot(self, first_slot);
                TValue* second = stack_slot(self, second_slot);
                if (is_numberic(first) && is_numberic(second)) {
                    NUMBERIC_CMP_OP(<=);
                } else if (is_str(first) && is_str(second)) {
                    auto x = Str_cmp(tv2str(first), tv2str(second));
                    *stack_slot(self, result_slot) = {
                        .type = Type::Bool,
                        .b = x <= 0,
                    };
                } else {
                    Thread_throw(self, E::OP_NUM);
                }

                break;
            }

            case Opcode::GE: {
                StackIndex result_slot = INS_EF(ins);
                StackIndex first_slot = INS_AB(ins);
                StackIndex second_slot = INS_CD(ins);

                TValue* first = stack_slot(self, first_slot);
                TValue* second = stack_slot(self, second_slot);
                if (is_numberic(first) && is_numberic(second)) {
                    NUMBERIC_CMP_OP(>=);
                } else if (is_str(first) && is_str(second)) {
                    auto x = Str_cmp(tv2str(first), tv2str(second));
                    *stack_slot(self, result_slot) = {
                        .type = Type::Bool,
                        .b = x >= 0,
                    };
                } else {
                    Thread_throw(self, E::OP_NUM);
                }

                break;
            }

            case Opcode::GREATE: {
                StackIndex result_slot = INS_EF(ins);
                StackIndex first_slot = INS_AB(ins);
                StackIndex second_slot = INS_CD(ins);

                TValue* first = stack_slot(self, first_slot);
                TValue* second = stack_slot(self, second_slot);
                if (is_numberic(first) && is_numberic(second)) {
                    NUMBERIC_CMP_OP(>);
                } else if (is_str(first) && is_str(second)) {
                    auto x = Str_cmp(tv2str(first), tv2str(second));
                    *stack_slot(self, result_slot) = {
                        .type = Type::Bool,
                        .b = x == 1,
                    };
                } else {
                    Thread_throw(self, E::OP_NUM);
                }

                break;
            }

            case Opcode::EQ: {
                StackIndex result_slot = INS_EF(ins);
                StackIndex first_slot = INS_AB(ins);
                StackIndex second_slot = INS_CD(ins);

                TValue* first = stack_slot(self, first_slot);
                TValue* second = stack_slot(self, second_slot);
                if (is_numberic(first) && is_numberic(second)) {
                    NUMBERIC_CMP_OP(==);
                } else if (is_str(first) && is_str(second)) {
                    auto x = Str_equal(tv2str(first), tv2str(second));
                    *stack_slot(self, result_slot) = {
                        .type = Type::Bool,
                        .b = x,
                    };
                } else {
                    Thread_throw(self, E::OP_NUM);
                }

                break;
            }

            case Opcode::NE: {
                StackIndex result_slot = INS_EF(ins);
                StackIndex first_slot = INS_AB(ins);
                StackIndex second_slot = INS_CD(ins);

                TValue* first = stack_slot(self, first_slot);
                TValue* second = stack_slot(self, second_slot);
                if (is_numberic(first) && is_numberic(second)) {
                    NUMBERIC_CMP_OP(!=);
                } else if (is_str(first) && is_str(second)) {
                    auto x = !Str_equal(tv2str(first), tv2str(second));
                    *stack_slot(self, result_slot) = {
                        .type = Type::Bool,
                        .b = x,
                    };
                } else {
                    Thread_throw(self, E::OP_NUM);
                }

                break;
            }

            default: {
                fprintf(stderr,
                        "unsupport bytecode %s\n",
                        opcode_names[(int)INS_OP(ins)]);
                exit(1);
            }
        }
    }
    // NEVER REACH
    return 0;
}

static void Thread_call(Thread* self,
                        StackIndex func_i,
                        StackIndex desire_retvals_nr)
{
    auto tv_func = stack_slot(self, func_i);
    auto args = tv_func + 1;
    StackIndex args_nr = self->top - args;

    Thread_push_pc(self, self->pc);
    Thread_push_index(self, self->base - self->stack);
    Thread_push_index(self, self->top - self->stack);

    NF_CHECK(self, tv_func->type == Type::Func, "not a func");
    auto func = obj2func(tv2obj(tv_func));

    self->base = self->top;
    func->prev = self->func;
    self->func = func;

    auto desire_args_nr = args_nr;
    if (func->func_type == FuncType::NF) {
        desire_args_nr = func->proto->args_nr;
    }

    for (int i = 0; i < args_nr; i++) {
        Thread_push(self, args++);
    }

    if (desire_args_nr > args_nr) {
        for (int i = args_nr; i < desire_args_nr; i++) {
            Thread_push_nil(self);
        }
    } else {
        self->top -= (args_nr - desire_args_nr);
    }

    StackIndex retn = 0;

    if (func->func_type == FuncType::NF) {
        self->pc = func->proto->ins;
        retn = __Thread_run(self);
    } else {
        auto c_func = func->c_func;
        retn = c_func(self);
    }
    __Thread_return(self, func_i, desire_retvals_nr, retn);
}

Error Thread_pcall(Thread* self, ProtectedFunc f, void* ud)
{
    auto old_top = self->top - self->stack;
    auto old_base = self->base - self->stack;
    auto old_pc = self->pc;

    auto err = Thread_run_protected(self, f, ud);
    if (err != E::OK) {
        self->top = self->stack + old_top;
        self->base = self->stack + old_base;
        self->pc = old_pc;
    }
    return err;
}

void Thread_run(Thread* self, const char* code)
{
    auto err = Thread_load(self, code, strlen(code), "no_name");
    if (E::OK == err) {
        Thread_call(self, -1, 0);
    } else {
        Thread_throw(self, err);
    }
}

template <Size N>
class ArrayBuf {
public:
    char base[N];
    static constexpr Size size = N;
};

struct LoadFile {
    ArrayBuf<1024> buf;
    FILE* fp;
};

static const char* FILE_read(Thread* th, void* ud, size_t* size)
{
    (void)th;

    LoadFile* loadf = (LoadFile*)ud;
    if (feof(loadf->fp))
        return nullptr;
    auto read_n = fread(loadf->buf.base, 1, loadf->buf.size, loadf->fp);
    *size = read_n;
    return loadf->buf.base;
}

void Thread_run(Thread* self, FILE* fp)
{
    LoadFile load;
    load.fp = fp;
    auto err = Thread_load(self, FILE_read, &load, "stdin");
    if (E::OK == err) {
        Thread_call(self, -1, 0);
    } else {
        Thread_throw(self, err);
    }
}

void Thread_push_nil(Thread* self) { Thread_push(self, &nil); }

void Thread_push(Thread* self, bool b)
{
    TValue v = { .type = Type::Bool, .b = b };
    Thread_push(self, &v);
}

void Thread_push(Thread* self, Integer i)
{
    TValue v = { .type = Type::Integer, .i = i };
    Thread_push(self, &v);
}

void Thread_push(Thread* self, const TValue* tv)
{
    NF_CHECK(self, self->stack_nr + 1 <= MAX_STACK_NR, "stack overflow");
    if (self->stack_nr <= (self->top - self->stack)) {
        self->stack = NF_REALLOC_ARRAY_P(
            self, self->stack, TValue, self->stack_nr * 3 / 2 + 16);
        self->stack_nr = self->stack_nr * 3 / 2 + 16;
    }
    *(self->top++) = *tv;
}

void Thread_push_func(Thread* self, Func* f)
{
    TValue tv = { .type = Type::Func, .obj = f };
    Thread_push(self, &tv);
}

void Thread_push_index(Thread* self, StackIndex index)
{
    TValue tv = { .type = Type::Index, .index = index };
    Thread_push(self, &tv);
}

StackIndex Thread_pop_index(Thread* self) { return (--self->top)->index; }

void Thread_push_pc(Thread* self, const Instruction* pc)
{
    TValue tv = { .type = Type::PC, .pc = pc };
    Thread_push(self, &tv);
}

const Instruction* Thread_pop_pc(Thread* self) { return (--self->top)->pc; }

} // namespace nf::imp
