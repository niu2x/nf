#include "lexer.h"
#include "vm.h"

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
        CASE(SYMBOL)
    }

#undef CASE

    return "TODO:unknown";
}

void token_dump(const Token* self, FILE* fp)
{
    fprintf(fp, "Token(type: %s, value: ", token_type_str(self->type));
    switch (self->type) {
        case NF_TK_INTEGER: {
            int64_t i = 0;
            VM::main()->lookup_constant(self->value.constant_index, &i);
            fprintf(fp, "%ld", i);
            break;
        }
        case NF_TK_DOUBLE: {
            double d = 0;
            VM::main()->lookup_constant(self->value.constant_index, &d);
            fprintf(fp, "%lf", d);
            break;
        }
    }

    fprintf(fp, ")\n");
}

} // namespace nf