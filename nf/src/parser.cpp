#include "api.h"
#include "zio.h"

namespace nf {

struct SParser {
    ZIO* z;
    MBuffer buff;
    const char* name;
};

Error protected_parser(Thread* th, ZIO* z, const char* name)
{
    struct SParser p;
    Error err;
    p.z = z;
    p.name = name;
    MBuffer_init(&p.buff);
    // err = Thread_pcall(th, f_parser, &p, savestack(th, th->top),
    // th->errfunc);
    MBuffer_free(&p.buff);
    return err;
}

static void f_parser(Thread* th, void* ud)
{
    // int i;
    // Proto* tf;
    // Closure* cl;
    // struct SParser *p = cast(struct SParser *, ud);
    // int c = luaZ_lookahead(p->z);
    // luaC_checkGC(L);
    // tf = ((c == LUA_SIGNATURE[0]) ? luaU_undump : luaY_parser)(L, p->z,
    //                                                            &p->buff,
    //                                                            p->name);
    // cl = luaF_newLclosure(L, tf->nups, hvalue(gt(L)));
    // cl->l.p = tf;
    // for (i = 0; i < tf->nups; i++)  /* initialize eventual upvalues */
    //   cl->l.upvals[i] = luaF_newupval(L);
    // setclvalue(L, L->top, cl);
    // incr_top(L);
}

} // namespace nf