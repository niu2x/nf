#ifndef NF_VM_H
#define NF_VM_H

#include <string>
#include <map>
#include <set>
#include <vector>
#include <memory>

namespace nf {

class Package {
};

using PackageName = std::string;
using PackagePtr = std::unique_ptr<Package>;

class VM {
public:
    VM();
    ~VM();
    load(FILE* fp);

private:
    std::map<PackageName, PackagePtr> packages_;
};

} // namespace nf

#endif