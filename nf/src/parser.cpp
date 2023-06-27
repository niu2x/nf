#include "api.h"

namespace nf {

struct SParser {
    ZIO* z;
    Mbuffer buff;
    const char* name;
};

Error protected_parser(Thread* th, ZIO* z, const char* name)
{
    struct SParser p;
    Error err;
    p.z = z;
    p.name = name;
    luaZ_initbuffer(L, &p.buff);
    status = luaD_pcall(L, f_parser, &p, savestack(L, L->top), L->errfunc);
    luaZ_freebuffer(L, &p.buff);
    return status;
}

} // namespace nf