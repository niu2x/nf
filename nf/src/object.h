#ifndef NF_OBJECT_H
#define NF_OBJECT_H

#include <setjmp.h>

#include <cstdint>
#include <cstddef>

#include "utils.h"
#include "api.h"
#include "zio.h"
#include "basic_types.h"

namespace nf::imp {

struct Object {
    Object* next;
    Type type;
    Flags flags;
};

struct TValue {
    Type type;
    union {
        Integer i;
        Number n;

        Object* obj;
        const Instruction* pc;
        StackIndex index;
    };
};

int TValue_compare(TValue* a, TValue* b);

struct Str : Object {
    char* base;
    Size nr;
    Hash hash;
    // Hash sid;
};

struct Node {
    Node* next;
    TValue key;
    TValue value;
};

struct Table : Object {
    TValue* array_ptr;
    Integer array_alloc;

    Node** node_ptr;
    Size node_alloc;
    Size node_count;
};

Table* Table_new(Thread* th);
void Table_init(Table* self);
TValue* Table_set(Thread* th, Table* self, Integer index);
TValue* Table_set(Thread* th, Table* self, TValue* key, bool only_hash = false);

TValue* Table_get(Thread* th, Table* self, Integer index);
TValue* Table_get(Thread* th, Table* self, TValue* key, bool only_hash = false);

struct Scope {
    const char* var_names[MAX_VAR_NR];
    StackIndex var_slots[MAX_VAR_NR];
    uint8_t flags[MAX_VAR_NR];
    Size nr;
    Scope* parent;
    Thread* th;
};

union UpValuePos {
    struct {
        union {
            StackIndex parent_frame_slot;
            StackIndex parent_uv_index;
        };
        bool is_parent_uv;
    };
    uint32_t u32;
};

void Scope_init(Scope* self, Thread*);
StackIndex
Scope_search(Scope* self, const char*, uint8_t** flags, bool recursive = false);
void Scope_insert(Scope* self, const char*, StackIndex i);

NF_INLINE Size Scope_vars_nr(Scope* self, bool recursive = false)
{
    return self->nr
           + ((recursive && self->parent)
                  ? Scope_vars_nr(self->parent, recursive)
                  : 0);
}

NF_INLINE Size Scope_vars_nr2(Scope* self)
{
    if (self->nr == 0)
        return self->parent ? Scope_vars_nr2(self->parent) : 0;

    return self->var_slots[self->nr - 1] + 1;
}

struct UpValue : Object {
    bool closed;
    union {
        StackIndex abs_stack_index;
        TValue value;
    };
};

UpValue* UpValue_new(Thread* th, uint32_t uv_pos);

struct Proto : Object {
    Instruction* ins;
    Size ins_nr;
    Size ins_alloc;

    StackIndex used_slots;

    uint32_t up_values[MAX_UV_NR];
    StackIndex up_values_nr;

    ConstIndex const_nr;
    ConstIndex const_alloc;

    TValue* const_arr;

    StackIndex args_nr;

    Str* name;

    Scope* scope;
    Proto* parent;
};

UpValuePos
UpValue_search(Thread* th, Proto* proto, const char* name, StackIndex deep = 1);

enum class FuncType {
    C,
    NF,
};

struct Func : Object {
    FuncType func_type;
    union {
        struct {
            CFunc c_func;
        };
        struct {
            Proto* proto;
        };
    };

    UpValue* up_values[MAX_UV_NR];
    StackIndex up_values_nr;

    Func* prev;
};

Func* Func_new(Thread* th, Proto* proto);
Func* Func_new(Thread* th, CFunc cfunc);
Proto* Proto_new(Thread* th);

void Proto_append_ins(Thread* th, Proto* self, Instruction ins);
StackIndex Proto_insert_uv(Thread* th, Proto* self, UpValuePos uv_pos);
ConstIndex Proto_insert_const(Thread* th, Proto* self, TValue* v);
// StackIndex Proto_insert_uv(Thread* th, Proto* self, UpValuePos uv_pos);

struct StrTab {
    Str** buckets;
    Size buckets_nr;
};

struct GlobalState {
    // CFunction panic;
    TValue registry;
    Size total_bytes;
    Object* root;
    // Buffer buffer;

    StrTab str_tab;
};

// struct CallInfo {
//     TValue* base;
//     TValue* top;
//     TValue* func;
//     const Instruction* saved_pc;
//     int results;
//     int tailcalls;
// };

struct LongJmp {
    LongJmp* prev;
    jmp_buf b;
    Error status;
    const char* msg;
};

enum {
    ERROR_MSG_NR = 128,
};

struct Thread : Object {
    Error status;
    GlobalState* global;
    const Instruction* pc; /* `savedpc' of current function */

    TValue* top; // free slots
    TValue* base;

    TValue* stack;
    StackIndex stack_nr;

    StackIndex retvals_nr;

    TValue gt;
    struct LongJmp* error_jmp; /* current error recover point */

    Func* func;

    char error_msg[ERROR_MSG_NR];

    MBuffer tmp_buf;

    UpValue** up_values;
    uint64_t up_values_nr;
    uint64_t up_values_alloc;
    uint64_t closed_uv_nr;

    bool debug;
};

UpValue* Thread_search_opened_uv(Thread* th, StackIndex uv_pos);
void Thread_insert_opened_uv(Thread* th, UpValue* uv);

#define Thread_global(th)   (th->global)
#define Thread_registry(th) (&(th->global->registry))

#define obj2func(obj) ((Func*)(obj))
#define obj2proto(obj) ((Proto*)(obj))
#define obj2str(obj)  ((Str*)(obj))
#define obj2table(obj) ((Table*)(obj))

#define tv2obj(tv)   ((tv)->obj)
#define tv2str(tv)  obj2str(tv2obj(tv))
#define tv2proto(tv) obj2proto(tv2obj(tv))
#define tv2func(tv)  obj2func(tv2obj(tv))
#define tv2table(tv) obj2table(tv2obj(tv))

NF_INLINE void TValue_set_nil(TValue* tv) { tv->type = Type::NIL; }
NF_INLINE void TValue_set_table(TValue* tv, Table* obj)
{
    tv->type = Type::Table;
    tv->obj = obj;
}

} // namespace nf::imp

#endif
