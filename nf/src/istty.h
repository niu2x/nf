#ifndef NF_ISTTY_H
#define NF_ISTTY_H

#include <stdio.h>

namespace nf::imp {
bool istty(FILE*);
}

#endif