#ifndef NF_VM_H
#define NF_VM_H

#include <vector>
#include <cstdint>

#include "utils.h"

namespace nf {

class VM : private Noncopyable {
public:
    template <class T>
    using ConstantTable = std::vector<T>;

    VM();
    ~VM();

    static VM* main();

    int insert_constant(int64_t i);
    int insert_constant(double d);

    bool lookup_constant(int index, int64_t* out);
    bool lookup_constant(int index, double* out);

private:
    ConstantTable<int64_t> constant_integers_;
    ConstantTable<double> constant_doubles_;
};

} // namespace nf

#endif