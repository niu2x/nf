#include "api.h"
#include "zio.h"
#include "object.h"

namespace nf {

struct SParser {
    ZIO* z;
    MBuffer buff;
    const char* name;
};

static Proto* y_parser(Thread* th, ZIO* z, MBuffer* buff, const char* name)
{
    auto proto = Proto_new(th);
    return proto;
}

static void f_parser(Thread* th, void* ud)
{

    // int i;
    // Proto* tf;
    struct Proto* proto;
    struct Func* func;
    struct SParser* p = (struct SParser*)(ud);

    int c = ZIO_peek(p->z);
    // luaC_checkGC(L);
    // tf = ((c == LUA_SIGNATURE[0]) ? luaU_undump : luaY_parser)(L, p->z,
    //                                                            &p->buff,
    //                                                            p->name);
    proto = y_parser(th, p->z, &(p->buff), p->name);
    func = Func_new(th, proto);

    // cl = luaF_newLclosure(L, tf->nups, hvalue(gt(L)));
    // cl->l.p = tf;
    // for (i = 0; i < tf->nups; i++)  /* initialize eventual upvalues */
    //   cl->l.upvals[i] = luaF_newupval(L);
    // setclvalue(L, L->top, cl);
    // incr_top(L);
    //
    Thread_push_func(th, func);
}

Error protected_parser(Thread* th, ZIO* z, const char* name)
{
    struct SParser p;
    Error err;
    p.z = z;
    p.name = name;
    MBuffer_init(&p.buff);
    err = Thread_pcall(th, f_parser, &p);
    MBuffer_free(&p.buff);
    return err;
}

} // namespace nf