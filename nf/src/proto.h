#ifndef NF_VM_H
#define NF_VM_H

#include <vector>
#include <cstdint>
#include <string>
#include <memory>
#include <map>
#include <stack>
#include <sstream>

#include "utils.h"
#include "value.h"
#include "parser_visitor.h"

namespace nf {

class Proto {
public:
    template <class T>
    using ConstantTable = std::vector<T>;
    using UniqueConstStrPtr = std::unique_ptr<std::string>;
    using ConstantStrTable = ConstantTable<UniqueConstStrPtr>;

    Proto();
    ~Proto();

    NF_INLINE std::stringstream* edit() { return &edit_; }
    const uint8_t* codes() const { return (const uint8_t*)codes_.c_str(); }
    size_t codes_len() const { return codes_.size(); }
    void finish_edit();

    int insert_constant(int64_t i);
    int insert_constant(double d);
    int insert_constant(const char* sz);

    bool lookup_constant(int index, int64_t* out);
    bool lookup_constant(int index, double* out);
    bool lookup_constant(int index, const char** out);

private:
    std::string codes_;
    std::stringstream edit_;

    ConstantTable<int64_t> constant_integers_;
    ConstantTable<double> constant_doubles_;
    ConstantStrTable constant_strings_;
};

class ProtoBuilder : public ParserVisitor {
public:
    std::unique_ptr<Proto>
};

} // namespace nf

#endif