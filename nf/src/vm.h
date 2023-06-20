#ifndef NF_VM_H
#define NF_VM_H

#include <vector>
#include <cstdint>
#include <string>
#include <memory>
#include <map>
#include <stack>

#include "utils.h"

namespace nf {

class Package {
public:
    Package();
    ~Package();
    bool loading() const { return loading_; }
    const std::string& name() const { return name_; }
    void set_name(const std::string& name) { name_ = name; }

private:
    std::string name_;
    bool loading_;
};

class VM : private Noncopyable {
public:
    template <class T>
    using ConstantTable = std::vector<T>;
    using UniqueConstStrPtr = std::unique_ptr<const std::string>;
    using ConstantStrTable = ConstantTable<UniqueConstStrPtr>;
    using PackagePtr = std::unique_ptr<Package>;

    VM();
    ~VM();

    static VM* main();

    int insert_constant(int64_t i);
    int insert_constant(double d);
    int insert_constant(const char* sz);

    bool lookup_constant(int index, int64_t* out);
    bool lookup_constant(int index, double* out);
    bool lookup_constant(int index, const char** out);

    void push_new_package();

private:
    ConstantTable<int64_t> constant_integers_;
    ConstantTable<double> constant_doubles_;
    ConstantStrTable constant_strings_;
    std::map<std::string, PackagePtr> packages_;
    std::stack<PackagePtr> loading_packages_;
};

} // namespace nf

#endif