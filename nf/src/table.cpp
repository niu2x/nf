#include "object.h"

namespace nf::imp {
void Table_init(Table* self)
{
    self->array_ptr = nullptr;
    self->array_alloc = 0;

    self->node_ptr = nullptr;
    self->node_alloc = 0;
    self->node_count = 0;
}

static Hash hash_key(TValue* key)
{
    switch (key->type) {
        case Type::NIL:
            return 0;
        case Type::Number:
        case Type::Integer:
            return key->i;

        default:
            return (Hash)(key->obj);
    }
}

TValue* Table_set(Thread* th, Table* self, Integer index)
{
    if (index < self->array_alloc) {
        return &(self->array_ptr[index]);
    }

    TValue v = { .type = Type::Integer, .i = index };
    return Table_set(th, self, &v, true);
}

TValue* Table_set(Thread* th, Table* self, TValue* key, bool only_hash)
{
    if (!only_hash) {
        if (key->type == Type::Integer) {
            return Table_set(th, self, key->i);
        }
    }

    if (!self->node_alloc || self->node_count + 1 > self->node_alloc) {
        Size new_node_alloc = next_power_of_2(self->node_count + 1) - 1;
        Node** new_node_ptr = NF_ALLOC_ARRAY_P(th, Node*, new_node_alloc);
        memset(new_node_ptr, 0, sizeof(Node*) * new_node_alloc);

        for (Size i = 0; i < self->node_alloc; i++) {
            Node* node = self->node_ptr[i];
            while (node) {
                auto the = node;
                node = node->next;

                auto bucket = hash_key(&(the->key)) % new_node_alloc;
                the->next = new_node_ptr[bucket];
                new_node_ptr[bucket] = the;
            }
        }

        if (self->node_alloc)
            NF_FREE(self->node_ptr);

        self->node_alloc = new_node_alloc;
        self->node_ptr = new_node_ptr;
    }

    Size bucket = hash_key(key) % self->node_alloc;
    Node* node = self->node_ptr[bucket];
    while (node) {
        if (!TValue_compare(&(node->key), key))
            return &(node->value);
        node = node->next;
    }

    node = NF_ALLOC_ARRAY_P(th, Node, 1);
    node->key = *key;
    node->value = { .type = Type::NIL, .n = 0 };
    node->next = self->node_ptr[bucket];
    self->node_ptr[bucket] = node;
    self->node_count++;

    return &(node->value);
}

int TValue_compare(TValue* a, TValue* b)
{
    if (a->type == b->type) {

        switch (a->type) {
            case Type::NIL:
                return 0;
            case Type::Integer: {
                auto diff = a->i - b->i;
                if (diff < 0)
                    return -1;
                else if (!diff)
                    return 0;
                return 1;
            }
            case Type::Number: {
                auto diff = a->n - b->n;
                if (diff < 0)
                    return -1;
                else if (!diff)
                    return 0;
                return 1;
            }
            default:
                // case Type::Table:
                // case Type::String:
                // case Type::Func:
                //
                {
                    auto diff = a->obj - b->obj;
                    if (diff < 0)
                        return -1;
                    else if (!diff)
                        return 0;
                    return 1;
                }
                return 0;
        }

    } else
        return ((int)(a->type)) - ((int)(b->type));
}

TValue* Table_get(Thread* th, Table* self, Integer index)
{
    if (index < self->array_alloc) {
        return &self->array_ptr[index];
    }

    TValue v = { .type = Type::Integer, .i = index };
    return Table_get(th, self, &v, true);
}

Table* Table_new(Thread* th)
{
    Table* self = NF_ALLOC_ARRAY_P(th, Table, 1);
    Table_init(self);
    return self;
}

TValue* Table_get(Thread* th, Table* self, TValue* key, bool only_hash)
{
    if (!only_hash) {
        if (key->type == Type::Integer) {
            return Table_get(th, self, key->i);
        }
    }

    if (!self->node_alloc)
        return nullptr;

    Size bucket = hash_key(key) % self->node_alloc;
    Node* node = self->node_ptr[bucket];
    while (node) {
        if (!TValue_compare(&(node->key), key))
            return &(node->value);
        node = node->next;
    }
    return nullptr;
}

} // namespace nf::imp