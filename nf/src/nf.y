%define parse.error verbose
%define api.push-pull push
%define api.pure
%parse-param {void* nf_parser} {void *userdata}

%token NF_TK_EOF
%token NF_TK_UNKNOWN
%token NF_TK_INTEGER
%token NF_TK_DOUBLE
%token NF_TK_STRING
%token NF_TK_SYMBOL
%token NF_TK_PRINT
%token NF_TK_PACKAGE

%{

#include <stdio.h>
#include <stdint.h>
#include "parser_visitor.h"
#include "nonterminal.h"

#define ENTER(xx) pl_enter(NF_NT_ ## xx)
#define EXIT(xx) pl_exit(NF_NT_ ## xx)

extern void nf_error(void* p, void*, const char*);

%}

%union {
	int constant_index;
};

%%

start: { ENTER(START); } block NF_TK_EOF { pl_token(NF_TK_EOF, yylval); } { EXIT(START); }

block: { ENTER(BLOCK); } stmts { EXIT(BLOCK); }

stmts: 
	|  { ENTER(STMTS);} stmts1  {EXIT(STMTS);}

stmts1: { ENTER(STMTS1);} stmt semis stmts { EXIT(STMTS1);}

stmt: { ENTER(STMT);} stmt_print { EXIT(STMT);}

stmt_print: { ENTER(STMT_PRINT);} NF_TK_PRINT NF_TK_INTEGER { EXIT(STMT_PRINT);}

semis:
	| semis1

semis1: ';' semis

%%