#include "lexer.h"

extern "C" {
void nf_error(void* p, void*, const char*) { }
}

namespace nf {

static const char* token_type_str(int type)
{

#define CASE(name)                                                             \
    case NF_TK_##name:                                                         \
        return #name;

    switch (type) {
        CASE(EOF)
        CASE(UNKNOWN)
        CASE(INTEGER)
        CASE(DOUBLE)
        CASE(STRING)
    }

#undef CASE

    return "TODO:unknown";
}

void token_dump(const Token* self, FILE* fp)
{
    fprintf(fp, "Token(%s)\n", token_type_str(self->type));
}

} // namespace nf