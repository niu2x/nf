#ifndef NF_MAIN_VM_C_API_H
#define NF_MAIN_VM_C_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int main_vm_insert_constant_interger(int64_t i);
int main_vm_insert_constant_double(double d);
int main_vm_insert_constant_string(const char* sz);

#ifdef __cplusplus
}
#endif

#endif