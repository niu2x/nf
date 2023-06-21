#include "main_vm_c_api.h"

#include <string.h>

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

int main_vm_insert_constant_string(const char* sz)
{
    return VM::main()->insert_constant(sz);
}

void main_vm_push_package() { VM::main()->push_package(); }
void main_vm_pop_package() { VM::main()->pop_package(); }
void main_vm_current_package_set_name(const char* name)
{
    VM::main()->current_package()->set_name(name);
}

void main_vm_push_sstream() { VM::main()->push_sstream(); }
void main_vm_pop_sstream() { VM::main()->pop_sstream(); }
void main_vm_current_sstream_append_string_constant(int str_index)
{
    const char* str = "";
    VM::main()->lookup_constant(str_index, &str);
    main_vm_current_sstream_append_string(str);
}

void main_vm_current_sstream_append_string(const char* str)
{
    VM::main()->current_sstream()->write(str, strlen(str));
}

void main_vm_current_package_set_name_as_current_sstream()
{
    VM::main()->current_package()->set_name(
        VM::main()->current_sstream()->str());
}
