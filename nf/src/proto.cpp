#include "proto.h"
#include <string.h>
#include <algorithm>
#include "bytecode.h"

namespace nf {

template <class T, class Value>
static int ConstantTable_insert(Proto::ConstantTable<T>* self, Value&& v)
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

static int ConstantTable_insert(Proto::ConstantStrTable* self, const char* sz)
{
    auto begin = self->begin();
    auto end = self->end();

    auto iter = std::find_if(
        begin, end, [sz](auto& x) { return strcmp(sz, x->c_str()) == 0; });

    if (iter == end) {
        auto item = std::make_unique<std::string>(sz);
        self->push_back(std::move(item));
        return self->size() - 1;
    }
    return iter - begin;
}

template <class T>
static bool ConstantTable_lookup(
    Proto::ConstantTable<T>* self, int index, T* out)
{
    if (index < self->size() && index >= 0) {
        *out = self->at(index);
        return true;
    }
    return false;
}

static bool ConstantTable_lookup(
    Proto::ConstantStrTable* self, int index, const char** out)
{
    if (index < self->size() && index >= 0) {
        *out = self->at(index)->c_str();
        return true;
    }
    return false;
}

Proto::Proto() { }
Proto::~Proto() { }

void Proto::finish_edit()
{
    codes_ = edit_.str();
    edit_.clear();
}

int Proto::insert_constant(int64_t i)
{
    return ConstantTable_insert(&constant_integers_, i);
}

int Proto::insert_constant(double d)
{
    return ConstantTable_insert(&constant_doubles_, d);
}

bool Proto::lookup_constant(int index, int64_t* out)
{
    return ConstantTable_lookup(&constant_integers_, index, out);
}
bool Proto::lookup_constant(int index, double* out)
{
    return ConstantTable_lookup(&constant_doubles_, index, out);
}

int Proto::insert_constant(const char* sz)
{
    return ConstantTable_insert(&constant_strings_, sz);
}

bool Proto::lookup_constant(int index, const char** out)
{
    return ConstantTable_lookup(&constant_strings_, index, out);
}

} // namespace nf