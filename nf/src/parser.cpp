

#include <stdio.h>
#include <ctype.h>

#include "api.h"
#include "zio.h"
#include "bytecode.h"
#include "object.h"

// #define printf(...)

namespace nf {

struct SParser {
    ZIO* z;
    MBuffer buff;
    const char* name;
};

union SemInfo {
    Integer i;
    Number n;
    Str* s;
};

static SemInfo null_seminfo = { .s = nullptr };

enum TokenType {
    TT_NONE = 256,
    TT_EOF,
    TT_INTEGER,
    TT_NUMBER,
    TT_SYMBOL,
    TT_STRING,
    TT_PRINT,
    TT_LOCAL,
    TT_EQ,
};

struct Keyword {
    const char* symbol;
    int token;
};

static Keyword keywords[] = {
    { "local", TT_LOCAL },
    { "print", TT_PRINT },
    { nullptr, 0 },
};

struct Token {
    int token;
    SemInfo seminfo;
};

static Token NONE = { .token = TT_NONE, .seminfo = { .s = nullptr } };

struct LexState {
    int current; /* current character (charint) */
    Token t; /* current token */
    Token peek; /* look ahead token */

    Thread* th;
    ZIO* z;
    MBuffer* buff;
};

struct FuncState {
    LexState* ls;
    Proto* proto;
    Scope* scope;
};

enum class SingleValueType {
    NONE,
    NORMAL,
    TABLE_SLOT,
};

struct SingleValue {
    SingleValueType type;
    StackIndex index;
};
static SingleValue single_value_none
    = { .type = SingleValueType::NONE, .index = 0 };

#define next_chr(ls) (ls)->current = ZIO_next((ls)->z)

static NF_INLINE Token token_build(int token)
{
    return Token { .token = token, .seminfo = null_seminfo };
}

static Token next_token(LexState* ls)
{
    auto buff = ls->buff;
    if (ls->current == EOF) {
        next_chr(ls);
    }

    while (true) {
        // printf("ls->current %d %c\n", ls->current, ls->current);
        switch (ls->current) {
            case EOF: {
                next_chr(ls);
                return token_build(TT_EOF);
            }

            case ';':
            case '[':
            case ']':
            case '{':
            case '}':
            case '(':
            case ')':
            case '*':
            case '/':
            case '-':
            case '+': {
                auto c = ls->current;
                next_chr(ls);
                return token_build(c);
            }

            case '=': {
                next_chr(ls);
                if (ls->current == '=') {
                    next_chr(ls);
                    return token_build(TT_EQ);
                } else
                    return token_build('=');
            }

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                Integer i = ls->current - '0';
                Number n;
                Number scale;
                TokenType tt = TT_INTEGER;
                next_chr(ls);

                while (true) {
                    if (tt == TT_INTEGER) {
                        if (ls->current >= '0' && ls->current <= '9') {
                            i = i * 10 + ls->current - '0';
                            next_chr(ls);
                        } else if (ls->current == '.') {
                            tt = TT_NUMBER;
                            n = i;
                            scale = 0.1;
                            next_chr(ls);
                        } else {
                            break;
                        }
                    } else {
                        if (ls->current >= '0' && ls->current <= '9') {
                            n = n + scale * (ls->current - '0');
                            scale *= 0.1;
                            next_chr(ls);
                        } else
                            break;
                    }
                }

                if (tt == TT_INTEGER)
                    return Token { .token = tt, .seminfo = { .i = i } };
                else
                    return Token { .token = tt, .seminfo = { .n = n } };
            }
            case '\t':
            case ' ':
            case '\r':
            case '\n': {
                next_chr(ls);
                break;
            }

            case '\'':
            case '"': {
                auto quote = ls->current;
                next_chr(ls);
                MBuffer_reset(buff);
                while (ls->current != quote) {
                    char ch = ls->current;
                    MBuffer_append(ls->th, buff, &ch, 1);
                    next_chr(ls);
                }
                next_chr(ls);
                auto str = Str_new(ls->th, buff->data, buff->nr);

                return Token { .token = TT_STRING, .seminfo = { .s = str } };
            }

            default: {
                if (ls->current == '_' || isalpha(ls->current)) {
                    char ch = ls->current;
                    MBuffer_reset(buff);
                    MBuffer_append(ls->th, buff, &ch, 1);
                    next_chr(ls);
                    // printf("ls->current %c %d\n", ls->current, ls->current);

                    while (ls->current == '_' || isalnum(ls->current)) {
                        char ch = ls->current;
                        MBuffer_append(ls->th, buff, &ch, 1);
                        next_chr(ls);
                        // printf("ls->current %c %d\n", ls->current,
                        // ls->current);
                    }

                    auto str = Str_new(ls->th, buff->data, buff->nr);

                    auto keyword = keywords;
                    while (keyword->symbol) {
                        if (!strcmp(keyword->symbol, str->base)) {
                            return token_build(keyword->token);
                        }
                        keyword++;
                    }

                    return Token { .token = TT_SYMBOL,
                        .seminfo = { .s = str } };
                } else {
                    Thread_throw(
                        ls->th, E::PARSE, "when next_token unexpected char");
                }
            }
        }
    }
    Thread_throw(ls->th, E::PARSE, "never happened");

    return NONE;
}

static NF_INLINE Token* peek(LexState* ls)
{
    if (ls->peek.token == TT_NONE) {
        ls->peek = next_token(ls);
    }
    return &(ls->peek);
}

static NF_INLINE void next(LexState* ls)
{
    if (ls->peek.token == TT_NONE) {
        ls->t = next_token(ls);
    } else {
        ls->t = ls->peek;
        ls->peek = NONE;
    }
}

static NF_INLINE void expect(LexState* ls, int token)
{
    if (peek(ls)->token == token) {
        next(ls);
    } else {
        printf("expect %c, it is %d\n", token, peek(ls)->token);
        Thread_throw(ls->th, E::PARSE, "expect error");
    }
}

static void assignment(FuncState* fs, SingleValue left_slot);

static void emit(FuncState* fs, Instruction ins, int slots_changed)
{
    Proto_append_ins(fs->ls->th, fs->proto, ins);
    fs->proto->used_slots += slots_changed;
}

static void emit_const(FuncState* fs, TValue* c)
{
    auto const_index = Proto_insert_const(fs->ls->th, fs->proto, c);
    next(fs->ls);
    emit(fs, INS_FROM_OP_AB(Opcode::CONST, const_index), 1);
}

static SingleValue expr(FuncState* fs);
static SingleValue single_value(FuncState* fs, SingleValue prev);

#define MAX_USED_SLOT(fs) ((StackIndex)((fs)->proto->used_slots - 1))

static SingleValue const_value(FuncState* fs)
{
    auto token = peek(fs->ls);
    switch (token->token) {
        case TT_INTEGER: {
            TValue value = { .type = Type::Integer, .i = token->seminfo.i };
            emit_const(fs, &value);
            break;
        }
        case TT_NUMBER: {
            TValue value = { .type = Type::Number, .n = token->seminfo.n };
            emit_const(fs, &value);
            break;
        }
        case TT_STRING: {
            TValue value = { .type = Type::String, .obj = token->seminfo.s };
            emit_const(fs, &value);
            break;
        }
    }
    return { .type = SingleValueType::NORMAL, .index = MAX_USED_SLOT(fs) };
}

static SingleValue table_value(FuncState* fs)
{
    next(fs->ls);
    expect(fs->ls, '}');
    emit(fs, INS_FROM_OP_NO_ARGS(Opcode::NEW_TABLE), 1);
    return { .type = SingleValueType::NORMAL, .index = MAX_USED_SLOT(fs) };
}

static SingleValue mul_or_div_elem(FuncState* fs)
{
    SingleValue value;
    auto token = peek(fs->ls);
    if (token->token == '(') {
        next(fs->ls);
        value = expr(fs);
        expect(fs->ls, ')');

    } else {
        value = single_value(fs, single_value_none);
        printf("value %d\n", value.index);
    }

    if (value.type == SingleValueType::NORMAL) {
        return value;
    } else if (value.type == SingleValueType::TABLE_SLOT) {
        printf("niu2x sssssssss\n");
        emit(fs, INS_FROM_OP_AB(Opcode::TABLE_GET, value.index), 0);
        return { .type = SingleValueType::NORMAL, .index = MAX_USED_SLOT(fs) };
    } else {
        // never reach
        return value;
    }
}

static SingleValue add_or_sub_elem(FuncState* fs)
{
    auto first = mul_or_div_elem(fs);
    while (true) {
        auto token = peek(fs->ls);
        switch (token->token) {
            case '*': {
                next(fs->ls);
                auto second = mul_or_div_elem(fs);
                emit(fs,
                    INS_FROM_OP_AB_CD(Opcode::MUL, first.index, second.index),
                    1);
                break;
            }
            case '/': {
                next(fs->ls);
                auto second = mul_or_div_elem(fs);
                emit(fs,
                    INS_FROM_OP_AB_CD(Opcode::DIV, first.index, second.index),
                    1);
                break;
            }
            default: {
                goto end_loop;
            }
        }
        first = { .type = SingleValueType::NORMAL, .index = MAX_USED_SLOT(fs) };
    }
end_loop:

    return first;
}

static SingleValue expr(FuncState* fs)
{
    auto first = add_or_sub_elem(fs);
    while (true) {
        auto token = peek(fs->ls);
        switch (token->token) {
            case '+': {
                next(fs->ls);
                auto second = add_or_sub_elem(fs);
                emit(fs,
                    INS_FROM_OP_AB_CD(Opcode::ADD, first.index, second.index),
                    1);
                break;
            }
            case '-': {
                next(fs->ls);
                auto second = add_or_sub_elem(fs);
                emit(fs,
                    INS_FROM_OP_AB_CD(Opcode::SUB, first.index, second.index),
                    1);
                break;
            }
            default: {
                goto end_loop;
            }
        }

        first = { .type = SingleValueType::NORMAL, .index = MAX_USED_SLOT(fs) };
    }
end_loop:

    return first;
}

static void stmt_print(FuncState* fs)
{
    auto result = expr(fs);
    emit(fs, INS_FROM_OP_AB(Opcode::PUSH, result.index), 1);
    emit(fs, INS_FROM_OP_NO_ARGS(Opcode::PRINT), -1);
}

static void optional_init_assignment(FuncState* fs, SingleValue left_value)
{
    auto token = peek(fs->ls);
    if (token->token == '=') {
        assignment(fs, left_value);
    }
}

static void stmt_local(FuncState* fs)
{
    auto token = peek(fs->ls);
    if (token->token != TT_SYMBOL) {
        Thread_throw(fs->ls->th, E::PARSE, "expect a symbol");
    }

    const char* var_name = token->seminfo.s->base;

    VarIndex var_index;
    StackIndex slot;

    if ((var_index = Scope_search(fs->scope, var_name)) < 0) {
        var_index = Scope_insert(fs->scope, var_name);
        slot = fs->proto->used_slots;
        NF_CHECK(
            fs->ls->th, slot == var_index, "slot should equal to var_index");
        // fs->scope->var_slots[var_index] = slot;
        emit(fs, INS_FROM_OP_NO_ARGS(Opcode::LOAD_NIL), 1);

    } else {
        // slot = fs->scope->var_slots[var_index];
        slot = var_index;
    }

    next(fs->ls);

    optional_init_assignment(
        fs, { .type = SingleValueType::NORMAL, .index = slot });
}

// static void single_value(FuncState* fs)
// {
//     auto token = peek(fs->ls);
//     if (token->token == TT_SYMBOL) {
//         Index var_index = Scope_search(fs->scope, token->seminfo.s->base);
//         if (var_index < 0) {
//             emit(fs, INS_FROM_OP_NO_ARGS(Opcode::LOAD_NIL), 1);
//             next(fs->ls);
//         } else {
//             auto slot = fs->scope->var_slots[var_index];
//             next(fs->ls);
//             token = peek(fs->ls);

//             if (token->token == '[') {
//                 next(fs->ls);
//                 expr(fs);
//                 expect(fs->ls, ']');
//                 emit(fs, INS_FROM_OP_AB(Opcode::TABLE_GET, slot), 0);
//             } else {
//                 emit(fs, INS_FROM_OP_ABCDEF(Opcode::PUSH, slot), 1);
//             }
//         }

//     } else if (token->token == '{') {
//         table_value(fs);
//     } else {
//         const_value(fs);
//     }
// }

static SingleValue single_value(FuncState* fs, SingleValue prev)
{
    auto token = peek(fs->ls);

    switch (prev.type) {
        case SingleValueType::NONE: {
            if (token->token == TT_SYMBOL) {
                VarIndex var_index;
                StackIndex slot;
                auto var_name = token->seminfo.s->base;
                if ((var_index = Scope_search(fs->scope, var_name)) < 0) {
                    Thread_throw(
                        fs->ls->th, E::PARSE, "undefine var is not left_value");
                }
                slot = var_index;
                next(fs->ls);

                return single_value(
                    fs, { .type = SingleValueType::NORMAL, .index = slot });
            } else if (token->token == '{') {
                return single_value(fs, table_value(fs));
            } else if (token->token == TT_INTEGER || token->token == TT_NUMBER
                || token->token == TT_STRING) {
                return single_value(fs, const_value(fs));
            } else {
                Thread_throw(
                    fs->ls->th, E::PARSE, "when parse left_value error");
            }

            break;
        }

        case SingleValueType::NORMAL: {
            if (token->token == '[') {
                next(fs->ls);
                auto key = expr(fs);
                emit(fs, INS_FROM_OP_AB(Opcode::PUSH, key.index), 1);
                expect(fs->ls, ']');
                return single_value(fs,
                    { .type = SingleValueType::TABLE_SLOT,
                        .index = prev.index });
            }
            break;
        }
        case SingleValueType::TABLE_SLOT: {
            if (token->token == '[') {
                next(fs->ls);
                emit(fs, INS_FROM_OP_AB(Opcode::TABLE_GET, prev.index), 0);
                auto slot = MAX_USED_SLOT(fs);
                prev.index = slot;
                auto key = expr(fs);
                emit(fs, INS_FROM_OP_AB(Opcode::PUSH, key.index), 1);
                expect(fs->ls, ']');
                return single_value(fs, prev);
            }

            break;
        }
    }

    return prev;
}

static void assignment(FuncState* fs, SingleValue left_value)
{
    auto key_slot = MAX_USED_SLOT(fs);
    expect(fs->ls, '=');
    auto result = expr(fs);
    emit(fs, INS_FROM_OP_AB(Opcode::PUSH, result.index), 1);

    if (left_value.type == SingleValueType::NORMAL) {
        emit(fs, INS_FROM_OP_AB(Opcode::SET, left_value.index), -1);
    } else if (left_value.type == SingleValueType::TABLE_SLOT) {
        emit(fs,
            INS_FROM_OP_AB_CD(Opcode::TABLE_SET, left_value.index, key_slot),
            -1);
    }
}

static bool stmt(FuncState* fs)
{
    auto token = peek(fs->ls);
    bool chunk_finished = false;

    switch (token->token) {
        case TT_EOF: {
            chunk_finished = true;
            break;
        }
        case TT_PRINT: {
            next(fs->ls);
            stmt_print(fs);
            break;
        }

        case TT_LOCAL: {
            next(fs->ls);
            stmt_local(fs);
            break;
        }

        case TT_SYMBOL: {
            auto left_slot = single_value(fs, single_value_none);
            assignment(fs, left_slot);
            break;
        }

        default: {
            Thread_throw(fs->ls->th, E::PARSE, "when stmt, unexpected token");
        }
    }

    Size tmp_nr = fs->proto->used_slots - Scope_vars_nr(fs->scope);
    emit(fs, INS_FROM_OP_AB(Opcode::POP, tmp_nr), -tmp_nr);

    return chunk_finished;
}

static bool stmt_with_semi(FuncState* fs)
{
    bool chunk_finished = stmt(fs);
    if (!chunk_finished)
        expect(fs->ls, ';');
    return chunk_finished;
}

static void chunk(FuncState* fs)
{
    Scope scope;
    Scope_init(&scope, fs->ls->th);

    scope.parent = fs->scope;
    fs->scope = &scope;

    bool chunk_finished = stmt_with_semi(fs);
    while (!chunk_finished) {
        chunk_finished = stmt_with_semi(fs);
    }
    emit(fs, INS_FROM_OP_NO_ARGS(Opcode::RET_0), 0);
}

static Proto* y_parser(Thread* th, ZIO* z, MBuffer* buff, const char* name)
{
    LexState ls;
    ls.th = th;
    ls.z = z;
    ls.buff = buff;
    ls.t = NONE;
    ls.peek = NONE;
    ls.current = EOF;

    FuncState fs;
    fs.ls = &ls;
    fs.proto = nullptr;
    fs.scope = nullptr;

    auto proto = Proto_new(th);
    fs.proto = proto;
    proto->name = Str_new(th, name, strlen(name));

    chunk(&fs);

    return fs.proto;
}

static void f_parser(Thread* th, void* ud)
{

    struct Proto* proto;
    struct Func* func;
    struct SParser* p = (struct SParser*)(ud);

    proto = y_parser(th, p->z, &(p->buff), p->name);
    func = Func_new(th, proto);
    Thread_push_func(th, func);
}

Error protected_parser(Thread* th, ZIO* z, const char* name)
{
    struct SParser p;
    Error err;
    p.z = z;
    p.name = name;
    MBuffer_init(&p.buff);
    err = Thread_pcall(th, f_parser, &p);
    MBuffer_free(&p.buff);
    return err;
}

void Scope_init(Scope* self, Thread* th)
{
    self->nr = 0;
    self->parent = nullptr;
    self->th = th;
}

VarIndex Scope_search(Scope* self, const char* name, bool recursive)
{
    for (Size i = 0; i < self->nr; i++) {
        if (self->var_names[i] == name)
            return i;
    }

    if (recursive && self->parent)
        return Scope_search(self->parent, name, recursive);

    return -1;
}

VarIndex Scope_insert(Scope* self, const char* name)
{
    if (self->nr == MAX_VAR_NR)
        Thread_throw(self->th, E::PARSE, "Too many vars");

    self->var_names[self->nr++] = name;
    return self->nr - 1;
}

} // namespace nf