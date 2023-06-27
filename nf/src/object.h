#ifndef NF_OBJECT_H
#define NF_OBJECT_H

#include <cstdint>
#include <cstddef>

#include "utils.h"
#include "api.h"
#include "basic_types.h"
#include "array.h"

namespace nf {

struct Object {
    Object* next;
    Type type;
    Flags flags;
};

union Value {
    Integer i;
    Number n;
    Object* obj;
};

struct TValue {
    Type type;
    Value value;
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

struct GlobalState {
    // CFunction panic;
    TValue registry;
    Size total_bytes;
    Object* root;
    // Buffer buffer;
};

struct CallInfo {
    TValue* base;
    TValue* top;
    TValue* func;
    const Instruction* saved_pc;
    int results;
    int tailcalls;
};

struct LongJmp {
    LongJmp* prev;
    jmp_buf b;
    Error status;
};

struct State : Object {
    Error status;
    GlobalState* global;
    const Instruction* saved_pc; /* `savedpc' of current function */

    TValue* top; // free slots
    TValue* base;

    // TValue* stack;
    // Size stack_alloc;
    // Size stack_nr;
    Array<TValue> stack;
    Array<CallInfo> call_infos;

    TValue gt;
    struct LongJmp* error_jmp; /* current error recover point */
};

#define State_global(state)   (state->global)
#define State_registry(state) (&(state->global->registry))

} // namespace nf

#endif
