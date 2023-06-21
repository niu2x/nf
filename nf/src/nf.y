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
#include "main_vm_c_api.h"
//#define LISP ((lisp_t*)userdata)

extern void nf_error(void* p, void*, const char*);

%}

%union {
	int constant_index;
};

%%

start: file_block NF_TK_EOF

file_block: { main_vm_push_package(); } package_declare stmts { main_vm_pop_package(); }

package_declare: NF_TK_PACKAGE { main_vm_push_sstream(); } package_name { main_vm_pop_sstream(); } semis

package_name: NF_TK_SYMBOL { main_vm_current_sstream_append_string_constant($<constant_index>1); } more_package_name_fields {main_vm_current_package_set_name_as_current_sstream();}

more_package_name_fields: 
	| '.' { main_vm_current_sstream_append_string("."); } NF_TK_SYMBOL { main_vm_current_sstream_append_string_constant($<constant_index>3); }  more_package_name_fields

semis: 
	| semis1

semis1: ';' semis

stmts: 
	| stmts1

stmts1: stmt stmts

stmt: NF_TK_PRINT NF_TK_INTEGER

%%