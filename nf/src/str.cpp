#include <string.h>

#include "api.h"
#include "object.h"

namespace nf {

Str* Str_new(Thread* th, const char* ptr, Size nr)
{
    auto str_tab = &(Thread_global(th)->str_tab);
    Hash hash = 0;
    auto str = StrTab_search(str_tab, ptr, nr, hash);
    if (!str) {
        str = (Str*)NF_ALLOC_P(th, sizeof(Str));
        str->type = Type::String;
        str->base = NF_ALLOC_ARRAY_P(th, char, nr + 1);
        str->nr = nr;
        str->hash = hash;
        memcpy(str->base, ptr, nr);
        str->base[nr] = 0;

        StrTab_insert(th, str_tab, str);
    }

    return str;
}

} // namespace nf