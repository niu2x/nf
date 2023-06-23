#ifndef NF_MAIN_VM_C_API_H
#define NF_MAIN_VM_C_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "nf_parser.h"

int pl_constant_integer(int64_t i);
int pl_constant_double(double d);
int pl_constant_string(const char* sz);
void pl_enter(int nonterminal_symbol);
void pl_exit(int nonterminal_symbol);
void pl_token(int token, YYSTYPE value);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
class ParserVisitor {
public:
    virtual void enter(int nonterminal_symbol) = 0;
    virtual void exit(int nonterminal_symbol) = 0;
    virtual void token(int token, YYSTYPE value) = 0;
    virtual int constant_integer(int64_t i) = 0;
    virtual int constant_double(double d) = 0;
    virtual int constant_string(const char* sz) = 0;
};

void pl_set_visitor(ParserVisitor* visitor);
#endif

#endif