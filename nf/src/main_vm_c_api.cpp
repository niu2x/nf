#include "main_vm_c_api.h"
#include "vm.h"

using namespace nf;

int main_vm_insert_constant_interger(int64_t i)
{
    return VM::main()->insert_constant(i);
}

int main_vm_insert_constant_double(double d)
{
    return VM::main()->insert_constant(d);
}