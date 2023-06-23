#ifndef NF_VALUE_H
#define NF_VALUE_H

namespace nf {

enum class ValueType {
    INTEGER,
    DOUBLE,
};

struct Value {
    ValueType type;
    union {
        int64_t i;
        double d;
    };
};

} // namespace nf

#endif