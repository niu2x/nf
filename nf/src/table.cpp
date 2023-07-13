#include "object.h"

namespace nf {
void Table_init(Table* self)
{
    self->array_ptr = nullptr;
    self->array_alloc = 0;

    self->node_ptr = nullptr;
    self->node_alloc = 0;
}

// TValue* Table_set(Thread* th, Table* self, Index index) {

// }

// TValue* Table_set(Thread* th, Table* self, TValue* key) {

// }

static Hash hash_key(TValue* key) { return 0; }

TValue* Table_get(Thread* th, Table* self, Index index)
{
    if (self->array_alloc > index)
        return &self->array_ptr[index];

    TValue v = { .type = Type::Integer, .i = index };
    return Table_get(th, self, &v, true);
}

TValue* Table_get(Thread* th, Table* self, TValue* key, bool only_hash)
{
    if (!only_hash) {
        if (key->type == Type::Integer) {
            auto value = Table_get(th, self, key->i);
            if (value)
                return value;
        }
    }

    Index bucket = hash_key(key) % self->node_alloc;
    Node* node = self->node_ptr[bucket];
    while (node) {
        if (!TValue_compare(&(node->key), key))
            return &(node->value);
        node = node->next;
    }
    return nullptr;
}

} // namespace nf