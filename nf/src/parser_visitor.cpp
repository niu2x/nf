#include "parser_visitor.h"

#include <string.h>

#include "bytecode.h"

using namespace nf;

static ParserVisitor* current_visitor;

int pl_constant_integer(int64_t i)
{
    return current_visitor->constant_integer(i);
}

int pl_constant_double(double d) { return current_visitor->constant_double(d); }

int pl_constant_string(const char* sz)
{
    return current_visitor->constant_string(sz);
}

void pl_set_visitor(ParserVisitor* visitor) { current_visitor = visitor; }

void pl_enter(int symbol) { current_visitor->enter(symbol); }
void pl_exit(int symbol) { current_visitor->exit(symbol); }
void pl_token(int token, YYSTYPE value)
{
    current_visitor->token(token, value);
}