#include "vm.h"
#include <algorithm>

namespace nf {

template <class T, class Value>
static int ConstantTable_insert(VM::ConstantTable<T>* self, Value&& v)
{
    auto begin = self->begin();
    auto end = self->end();
    auto iter = std::find(begin, end, v);
    if (iter == end) {
        self->push_back(std::forward<T>(v));
        return self->size() - 1;
    }
    return iter - begin;
}

template <class T>
static bool ConstantTable_lookup(VM::ConstantTable<T>* self, int index, T* out)
{
    if (index < self->size() && index >= 0) {
        *out = self->at(index);
        return true;
    }
    return false;
}

VM::VM() { }
VM::~VM() { }

int VM::insert_constant(int64_t i)
{
    return ConstantTable_insert(&constant_integers_, i);
}

int VM::insert_constant(double d)
{
    return ConstantTable_insert(&constant_doubles_, d);
}

bool VM::lookup_constant(int index, int64_t* out)
{
    return ConstantTable_lookup(&constant_integers_, index, out);
}
bool VM::lookup_constant(int index, double* out)
{
    return ConstantTable_lookup(&constant_doubles_, index, out);
}

static VM main_;
VM* VM::main() { return &main_; }

} // namespace nf