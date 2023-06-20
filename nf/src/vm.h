#ifndef NF_VM_H
#define NF_VM_H

#include <vector>
#include <cstdint>
#include <string>
#include <memory>

#include "utils.h"

namespace nf {

class VM : private Noncopyable {
public:
    template <class T>
    using ConstantTable = std::vector<T>;
    using UniqueConstStrPtr = std::unique_ptr<const std::string>;
    using ConstantStrTable = ConstantTable<UniqueConstStrPtr>;

    VM();
    ~VM();

    static VM* main();

    int insert_constant(int64_t i);
    int insert_constant(double d);
    int insert_constant(const char* sz);

    bool lookup_constant(int index, int64_t* out);
    bool lookup_constant(int index, double* out);
    bool lookup_constant(int index, const char** out);

private:
    ConstantTable<int64_t> constant_integers_;
    ConstantTable<double> constant_doubles_;
    ConstantStrTable constant_strings_;
};

} // namespace nf

#endif