%define parse.error verbose
%define api.push-pull push
%define api.pure
%parse-param {void* nf_parser} {void *userdata}

%token T_EOF

%{

#include <stdio.h>
#include <stdint.h>
//#define LISP ((lisp_t*)userdata)

%}

%union {
};

%%

start: T_EOF

%%