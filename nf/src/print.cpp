#include "api.h"
#include "object.h"

namespace nf::imp {

int print(Thread* th, TValue* value)
{
    NF_UNUSED(th);

    switch (value->type) {
        case Type::Integer: {
            printf("%ld\n", value->i);
            break;
        }
        case Type::Number: {
            printf("%lf\n", value->n);
            break;
        }
        case Type::String: {
            auto str = obj2str(tv2obj(value));
            fwrite(str->base, 1, str->nr, stdout);
            printf("\n");
            break;
        }
        case Type::NIL: {
            printf("nil\n");
            break;
        }
        case Type::Table: {
            printf("table(%p)\n", value->obj);
            break;
        }
        case Type::Func: {
            if (tv2func(value)->func_type == FuncType::C) {
                printf("c-func(%p)\n", value->obj);
            } else {
                printf("nf-func(%p)\n", value->obj);
            }
            break;
        }
        default: {
            printf("print it unsupport for %d", (int)(value->type));
        }
    }
    return 0;
}

} // namespace nf::imp