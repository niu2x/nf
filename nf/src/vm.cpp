#include "vm.h"
#include <string.h>
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

static int ConstantTable_insert(VM::ConstantStrTable* self, const char* sz)
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
static bool ConstantTable_lookup(VM::ConstantTable<T>* self, int index, T* out)
{
    if (index < self->size() && index >= 0) {
        *out = self->at(index);
        return true;
    }
    return false;
}

static bool ConstantTable_lookup(
    VM::ConstantStrTable* self, int index, const char** out)
{
    if (index < self->size() && index >= 0) {
        *out = self->at(index)->c_str();
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

int VM::insert_constant(const char* sz)
{
    return ConstantTable_insert(&constant_strings_, sz);
}

bool VM::lookup_constant(int index, const char** out)
{
    return ConstantTable_lookup(&constant_strings_, index, out);
}

void VM::push_package()
{
    auto package = std::make_unique<Package>();
    loading_packages_.push(std::move(package));
}
void VM::pop_package()
{
    auto package = std::move(loading_packages_.top());
    loading_packages_.pop();
    packages_[package->name()] = std::move(package);
}

void VM::push_sstream()
{
    auto ss = std::make_unique<StringStream>();
    string_streams_.push(std::move(ss));
}
void VM::pop_sstream() { string_streams_.pop(); }

void VM::dump() const
{
    printf("packages:\n");
    for (auto& item : packages_) {
        printf("\t%s\n", item.second->name().c_str());
    }
}

static VM main_;
VM* VM::main() { return &main_; }

} // namespace nf