%option noyywrap nodefault reentrant

%top {
	#include <stdint.h>

	#include "nf_parser.h"
	#include "strbuf.h"

	static strbuf_t* strbuf_get() {
		static strbuf_t strbuf;
		static uint8_t inited = 0;
		if(!inited) {
			inited = 1;
			strbuf_init(&strbuf, 256);
		}
		return &strbuf;
	} 
}


INTEGER [-+]?[0-9]+
DOUBLE  [-+]?[0-9]*.[0-9]+([eE][-+]?[0-9]+)?
STRING	\"([^\\"]|\.)*\"

%x STRING

%%

{INTEGER} {return NF_TK_INTEGER;}
{DOUBLE} {return NF_TK_DOUBLE;}

\" {
	BEGIN(STRING);
	strbuf_reset(strbuf_get());
}

<STRING>\" {
	BEGIN(INITIAL);
	strbuf_append_char(strbuf_get(), 0); 
	printf("NF_TK_STRING %s\n", strbuf_string(strbuf_get(), NULL));
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

<<EOF>> {return NF_TK_EOF;}

. {return NF_TK_UNKNOWN;}

%%