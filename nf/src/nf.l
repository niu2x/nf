%option noyywrap nodefault reentrant

%top {
	#include <stdint.h>

	#include "nf_parser.h"
	#include "strbuf.h"
	#include "main_vm_c_api.h"

	static strbuf_t* strbuf_get() {
		static strbuf_t strbuf;
		static uint8_t inited = 0;
		if(!inited) {
			inited = 1;
			strbuf_init(&strbuf, 256);
		}
		return &strbuf;
	} 

	#define yylval ((YYSTYPE *)yyget_extra(yyscanner))
}


INTEGER [-+]?[0-9]+
DOUBLE  [-+]?[0-9]*.[0-9]+([eE][-+]?[0-9]+)?
STRING	\"([^\\"]|\.)*\"
SYMBOL	[A-Za-z_]+[A-Za-z0-9_]* 


%x STRING

%%

PRINT {return NF_TK_PRINT;}
package {return NF_TK_PACKAGE;}

; {return ';';}
\. {return '.';}
\) {return ')';}
\( {return '(';}

{SYMBOL} {
	yylval->constant_index = main_vm_insert_constant_string(yytext);
	return NF_TK_SYMBOL;
}
{INTEGER} {
	int64_t i = atoll(yytext);
	yylval->constant_index = main_vm_insert_constant_interger(i);
	return NF_TK_INTEGER;
}

{DOUBLE} {
	double d = strtod(yytext, NULL);
	yylval->constant_index = main_vm_insert_constant_double(d);
	return NF_TK_DOUBLE;
}

\" {
	BEGIN(STRING);
	strbuf_reset(strbuf_get());
}

<STRING>\" {
	BEGIN(INITIAL);
	strbuf_append_char(strbuf_get(), 0); 
	yylval->constant_index = main_vm_insert_constant_string(strbuf_string(strbuf_get(), NULL));
	return NF_TK_STRING;
}

<STRING>. {
	if(*yytext != '\\') {
		strbuf_append_char(strbuf_get(), *yytext); 
	}
	else{
		char c = input(yyscanner);
		switch(c) {
			case 'n': {
				strbuf_append_char(strbuf_get(), '\n'); 
				break;
			}
			case 'r': {
				strbuf_append_char(strbuf_get(), '\r'); 
				break;
			}
			case 't': {
				strbuf_append_char(strbuf_get(), '\t'); 
				break;
			}
			default: {
				strbuf_append_char(strbuf_get(), c); 
			}
		}
	}
}

<STRING>\n {
	strbuf_append_char(strbuf_get(), *yytext); 
}

\n {}
\r {}
\t {}
" " {}

<<EOF>> {return NF_TK_EOF;}

. {return NF_TK_UNKNOWN;}

%%