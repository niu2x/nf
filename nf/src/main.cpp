#include <cxxopts.hpp>
#include <stdio.h>
#include <iostream>

#include <nf/config.h>

#include "lexer.h"

static void die() { exit(1); }

static cxxopts::Options build_command_args_parser()
{
    cxxopts::Options options("nf", "TODO: desc!");
    options.add_options()("v,version", "display version");
    options.add_options()("h,help", "display help");
    options.add_options()("only-lexer", "");
    options.add_options()("dump", "");
    return options;
}

static cxxopts::Options command_args_parser = build_command_args_parser();

static auto parse_command_args(int argc, char* argv[])
{
    try {
        auto result = command_args_parser.parse(argc, argv);
        return result;
    } catch (cxxopts::OptionException& e) {
        std::cout << "nf: unrecognized option" << std::endl;
        std::cout << command_args_parser.help();
        die();
        // never reach
        return command_args_parser.parse(argc, argv);
    }
}

static void display_version()
{
    printf("v%d.%d.%d\n", NF_VERSION_MAJOR, NF_VERSION_MINOR, NF_VERSION_PATCH);
}

static void load_file(FILE* fp)
{
    nf::Lexer lexer;
    nf::Parser parser;
    lexer.push_input(fp);
    nf::Token token;
    do {
        token = lexer.lex();
        bool succ = parser.parse(token, nullptr);
        if (!succ) {
            exit(1);
        }
    } while (token.type != NF_TK_EOF);
}

static void run(FILE* fp)
{
    load_file(fp);
    // nf::VM::main()->run();
}

int main(int argc, char* argv[])
{
    auto cmd_args = parse_command_args(argc, argv);
    auto opt_version = cmd_args["version"].as<bool>();
    if (opt_version) {
        display_version();
        return 0;
    };

    auto opt_help = cmd_args["help"].as<bool>();
    if (opt_help) {
        std::cout << command_args_parser.help();
        return 0;
    }

    auto opt_only_lexer = cmd_args["only-lexer"].as<bool>();
    if (opt_only_lexer) {
        nf::Lexer lexer;
        lexer.push_input(stdin);
        nf::Token token;
        do {
            token = lexer.lex();
            token_dump(&token, stdout);
        } while (token.type != NF_TK_EOF);
        return 0;
    }

    run(stdin);

    auto opt_dump = cmd_args["dump"].as<bool>();
    if (opt_dump) {
        // nf::VM::main()->dump();
    }

    return 0;
}