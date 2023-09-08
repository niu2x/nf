#include <iostream>
#ifdef _WIN32
    #include <io.h>
    #define ISATTY _isatty
    #define FILENO _fileno
#else
    #include <unistd.h>
    #define ISATTY isatty
    #define FILENO fileno
#endif

#include "istty.h"

namespace nf::imp {

bool istty(FILE* fp) { return ISATTY(FILENO(fp)); }

} // namespace nf::imp