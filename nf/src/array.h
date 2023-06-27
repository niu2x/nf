#ifndef NF_ARRAY_H
#define NF_ARRAY_H

#include "basic_types.h"

namespace nf {

template <class T>
class Array {
public:
    Array() { }
    ~Array() { }

    Array(const Array&) = delete;
    Array& operator=(const Array&) = delete;

private:
    T* data_;
    Size nr_;
    Size alloc_;
};

} // namespace nf

#endif