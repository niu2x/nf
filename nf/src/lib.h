#ifndef NF_LIB_H
#define NF_LIB_H

#include "object.h"

namespace nf::imp {

void lib_nf_init(Thread* self);
void lib_core_init(Thread* self);
}

#endif