#include <string.h>

#include "api.h"
#include "object.h"

namespace nf::imp {

// Str* Str_new_fmt(Thread* th, const char* fmt, ...)
// {
//     va_list ap;
//     va_start(ap, fmt);
//     auto s = Str_new_fmt(th, fmt, ap);
//     va_end(ap);
//     return s;
// }

Str* Str_new_fmt(Thread* th, const char* fmt, va_list ap)
{
    va_list copy_ap;
    va_copy(copy_ap, ap);
    auto size = vsnprintf(nullptr, 0, fmt, ap);
    MBuffer_reserve(th, &(th->tmp_buf), ++size);
    th->tmp_buf.nr = size;
    vsnprintf(th->tmp_buf.data, size, fmt, copy_ap);
    return Str_new(th, th->tmp_buf.data, size - 1);
}

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

Str* Str_concat(Thread* th, Str* a, Str* b)
{
    MBuffer_reset(&(th->tmp_buf));
    MBuffer_reserve(th, &(th->tmp_buf), a->nr + b->nr + 1);
    MBuffer_append(th, &(th->tmp_buf), a->base, a->nr);
    MBuffer_append(th, &(th->tmp_buf), b->base, b->nr);

    return Str_new(th, th->tmp_buf.data, a->nr + b->nr);
}

} // namespace nf::imp