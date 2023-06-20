%option noyywrap nodefault reentrant

%top {
	#include "nf_parser.h"
}

%%

\n {}
\r {}
\t {}

<<EOF>> {return NF_TK_EOF;}

. {return NF_TK_UNKNOWN;}

%%