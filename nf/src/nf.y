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
//#define LISP ((lisp_t*)userdata)

extern void nf_error(void* p, void*, const char*);

%}

%union {
	int constant_index;
};

%%

start: file_block NF_TK_EOF

file_block: package_declare stmts

package_declare: NF_TK_PACKAGE package_name semi

package_name: NF_TK_SYMBOL more_package_name_fields

more_package_name_fields: 
	| '.' NF_TK_SYMBOL more_package_name_fields

semi: 
	| ';'

stmts: 
	| stmts1

stmts1: stmt stmts

stmt: NF_TK_PRINT NF_TK_INTEGER

%%