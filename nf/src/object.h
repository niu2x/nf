#ifndef NF_OBJECT_H
#define NF_OBJECT_H

#include <setjmp.h>

#include <cstdint>
#include <cstddef>

#include "utils.h"
#include "api.h"
#include "basic_types.h"

namespace nf {

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
        Index index;
    };
};

NF_INLINE void TValue_set_nil(TValue* tv) { tv->type = Type::NIL; }

struct Str : Object {
    char* base;
    int nr;
    Hash hash;
};

struct Node {
    Node* next;
    TValue key;
    TValue value;
};

struct Table : Object {
    TValue* array_ptr;
    Size array_alloc;

    Node** node_ptr;
    Size node_alloc;
};

struct Proto : Object {
    Instruction* ins;
    Size ins_nr;
    Size var_nr;
};

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
};

Func* Func_new(Thread* th, Proto* proto);
Proto* Proto_new(Thread* th);

struct GlobalState {
    // CFunction panic;
    TValue registry;
    Size total_bytes;
    Object* root;
    // Buffer buffer;
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
};

struct Thread : Object {
    Error status;
    GlobalState* global;
    const Instruction* pc; /* `savedpc' of current function */

    TValue* top; // free slots
    TValue* base;

    TValue* stack;
    Size stack_nr;

    // CallInfo* ci_base;
    // Size ci_nr;
    // CallInfo* ci;

    TValue gt;
    struct LongJmp* error_jmp; /* current error recover point */
};

#define Thread_global(th)   (th->global)
#define Thread_registry(th) (&(th->global->registry))

#define obj2func(obj) ((Func*)(obj))
#define is_obj(type)
#define check(cond) true
#define tv2obj(tv)  (check(is_obj((tv)->type)), (tv)->obj)

} // namespace nf

#endif
