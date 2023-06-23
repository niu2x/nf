#ifndef NF_NONTERMINAL_H
#define NF_NONTERMINAL_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    NF_NT_START,
    NF_NT_BLOCK,
    NF_NT_SEMIS,
    NF_NT_SEMIS1,
    NF_NT_STMTS,
    NF_NT_STMTS1,
    NF_NT_STMT,
    NF_NT_STMT_PRINT,
};

#ifdef __cplusplus
}
#endif

#endif