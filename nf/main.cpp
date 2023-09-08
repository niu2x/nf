#include <cxxopts.hpp>
#include <stdio.h>
#include <iostream>

#include <nf/config.h>
#include <nf/api.h>

static void die() { exit(1); }

static cxxopts::Options build_command_args_parser()
{
    cxxopts::Options options("nf", "TODO: desc!");
    options.add_options()("v,version", "display version");
    options.add_options()("h,help", "display help");
    options.add_options()("debug", "debug");
    options.add_options()(
        "f,file", "script file pathname", cxxopts::value<std::string>());
    options.parse_positional({ "file" });
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
    printf(
        "nf v%d.%d.%d\n", NF_VERSION_MAJOR, NF_VERSION_MINOR, NF_VERSION_PATCH);
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

    auto opt_debug = cmd_args["debug"].as<bool>();

    auto th = nf::open();
    nf::set_debug(th, opt_debug);

    int exit_code = 0;

    if (cmd_args.count("file")) {
        auto filepath = cmd_args["file"].as<std::string>();
        FILE* fp = fopen(filepath.c_str(), "rb");
        if (fp) {
            nf::run(th, fp);
            fclose(fp);
        } else {
            std::cerr << "can not open " << filepath << std::endl;
            exit_code = 1;
        }
    } else {
        nf::run(th, stdin);
    }
    nf::close(th);
    return exit_code;
}