#ifndef NF_LEXER_H
#define NF_LEXER_H

#include <stdio.h>

#include <string>

extern "C" {
#include "nf_lexer.h"
#include "nf_parser.h"
}

#include "utils.h"

namespace nf {

struct Token {
    int type;
    YYSTYPE value;
};

class Lexer {
public:
    Lexer()
    {
        auto ret = nf_lex_init_extra(&token_value_, &scan_);
        if (ret != 0) {
            die("nf_lex_init_extra failed with error code: %dn", ret);
        }
        nf_set_in(nullptr, scan_);
    }
    virtual ~Lexer() { nf_lex_destroy(scan_); }

    void push_input(FILE* fp) { _push_input(fp); }
    void pop_input() { _pop_input(); }
    Token lex() { return _lex(); }

protected:
    virtual void _push_input(FILE* fp)
    {
        YY_BUFFER_STATE new_buffer = nf__create_buffer(fp, YY_BUF_SIZE, scan_);
        if (!new_buffer)
            die("OOM");
        nf_push_buffer_state(new_buffer, scan_);
    }
    virtual Token _lex()
    {
        return { .type = nf_lex(scan_), .value = token_value_ };
    }
    virtual void _pop_input() { nf_pop_buffer_state(scan_); }

private:
    yyscan_t scan_;
    YYSTYPE token_value_;
};

class Parser {
public:
    Parser()
    {
        state_ = nf_pstate_new();
        if (!state_)
            die("Parser::Parser state_ is nullptr.");
    }
    virtual ~Parser() { nf_pstate_delete(state_); }

    bool parse(const Token& token, void* userdata)
    {
        return _parse(token, userdata);
    }

protected:
    virtual bool _parse(const Token& token, void* userdata)
    {
        error_msg_ = "";
        auto ret
            = nf_push_parse(state_, token.type, &token.value, this, userdata);
        if (error_msg_.size() > 0) {
            fprintf(stderr, "error_msg: %s\n", error_msg_.c_str());
            return false;
        }
        if (ret == 1)
            return false;
        return true;
    }

private:
    void error(const char* msg) { error_msg_ += msg; }
    nf_pstate* state_;
    std::string error_msg_;
    friend void nf_error(void* p, void*, const char*);
};

void token_dump(const Token* self, FILE* fp);

} // namespace nf

#endif
