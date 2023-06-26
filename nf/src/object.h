#ifndef NF_OBJECT_H
#define NF_OBJECT_H

#include <cstdint>
#include <cstddef>

#include "utils.h"

namespace nf {

enum class Type : uint8_t {
    NIL,
    Integer,
    Number,
    Thread,
    Table,
    String,
};

using Flags = uint8_t;
using Hash = uint32_t;

using Integer = int64_t;
using Number = double;

using Size = size_t;
using Instruction = uint32_t;

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

struct State : Object {
    uint8_t status;
    TValue* top; // free slots
    TValue* base;
    GlobalState* global;
    const Instruction* saved_pc; /* `savedpc' of current function */

    TValue* stack;
    Size stack_alloc;
    Size stack_nr;

    CallInfo* ci;
    Size ci_nr;
    Size ci_alloc;

    TValue gt;
};

#define State_global(state)   (state->global)
#define State_registry(state) (&(state->global->registry))

} // namespace nf

#endif
