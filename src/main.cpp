#include <CLI11/CLI11.hpp>

#include <File.hpp>
#include <PE.hpp>
#include <cstdio>

int main(int argc, char* argv[])
{
    CLI::App app{"PEachy - a small PE (Portable Executable) file manipulator."};

    std::string input;

    CLI::App* list = app.add_subcommand(
        "list", "List the modules in the import section in load-order.");
    list->add_option("input", input, "Path to PE input.")->required();

    // CLI::App_p escalate = std::make_shared<CLI::App>("escalate");
    std::vector<std::string> dlls;
    CLI::App* escalate = app.add_subcommand(
        "escalate", "Escalate the loading order of an ordered list of DLLs.");
    escalate->add_option("input", input, "Path to PE input.")->required();
    bool dry_run = false;
    escalate->add_flag("-d,--dry-run",
                       dry_run,
                       "Emit the target import order without making changes.");
    escalate->add_option(
        "dlls",
        dlls,
        "Ordered space-separated list of DLLs to load as early as possible.");

    app.require_subcommand();

    CLI11_PARSE(app, argc, argv);

    bool writable = escalate->parsed() && !dry_run;

    File file;
    bool loaded = file.load(input, writable);

    if (!loaded)
    {
        return 1;
    }

    PE pe;
    if (!pe.load(file.data(), writable))
    {
        std::fprintf(stderr, "Input file is not a valid PE executable.");
        return 1;
    }

    if (*list)
    {
        pe.examine_imports();
    }
    else if (*escalate)
    {
        if (dry_run)
        {
            pe.escalate(dlls, nullptr);
        }
        else
        {
            pe.escalate(dlls, file.mutable_data());
        }
    }

    return 0;
}
