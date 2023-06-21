#ifndef NF_MAIN_VM_C_API_H
#define NF_MAIN_VM_C_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int main_vm_insert_constant_interger(int64_t i);
int main_vm_insert_constant_double(double d);
int main_vm_insert_constant_string(const char* sz);

void main_vm_push_package();
void main_vm_pop_package();
void main_vm_current_package_set_name(const char* name);
void main_vm_current_package_set_name_as_current_sstream();

void main_vm_push_sstream();
void main_vm_pop_sstream();
void main_vm_current_sstream_append_string_constant(int str_index);
void main_vm_current_sstream_append_string(const char* str);

#ifdef __cplusplus
}
#endif

#endif