%define parse.error verbose
%define api.push-pull push
%define api.pure
%parse-param {void* nf_parser} {void *userdata}

%token NF_TK_EOF
%token NF_TK_UNKNOWN

%{

#include <stdio.h>
#include <stdint.h>
//#define LISP ((lisp_t*)userdata)

extern void nf_error(void* p, void*, const char*);

%}

%union {
};

%%

start: NF_TK_EOF

%%