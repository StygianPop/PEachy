# 🍑 PEachy 🍑 

☢️ Alpha quality ☢️

PEachy is a small utility to quickly escalate DLLs in the import load order.
DLL load order can be important in cases where:

* DLLMain has side-effects that affect DLLs loaded in the future
* A DLL logs or otherwise interacts with DLL loading mechanisms
* A DLL-exported symbol (e.g. malloc/free) should take precedence over the same symbol available in other DLLs

```
peachy.exe escalate myexe.exe mimalloc.dll
```

## Usage

```
Usage: D:\PEachy\build\peachy.exe [OPTIONS] SUBCOMMAND

Options:
  -h,--help                   Print this help message and exit

Subcommands:
  list                        List the modules in the import section in load-order.
  escalate                    Escalate the loading order of an ordered list of DLLs.
```

### List

The `list` subcommand takes a single path to the input file, and lists the DLLs that will be loaded in order.

Example usage:

```
peachy.exe list .\peachy.exe
```

produces the following output:

```
Imports:

    KERNEL32.dll
    MSVCP140.dll
    VCRUNTIME140.dll
    VCRUNTIME140_1.dll
    api-ms-win-crt-runtime-l1-1-0.dll
    api-ms-win-crt-heap-l1-1-0.dll
    api-ms-win-crt-convert-l1-1-0.dll
    api-ms-win-crt-environment-l1-1-0.dll
    api-ms-win-crt-stdio-l1-1-0.dll
    api-ms-win-crt-filesystem-l1-1-0.dll
    api-ms-win-crt-math-l1-1-0.dll
    api-ms-win-crt-locale-l1-1-0.dll
```

### Escalate

The `escalate` subcommand takes a path to the input file, followed by a list of space-separated DLLs that should be resorted to the top.
The exe is rewritten in-place, sorting the import data directory to place the requested escalations in front, leaving
the remaining entries intact and in the same order provided.

An optional `--dry-run` (`-d` alias) flag may be supplied to simply print the results without modifying the executable.

Example usage (you'd never want to do this, but for illustrative purposes):

```
peachy.exe escalate .\myexe.exe VCRUNTIME140.dll api-ms-win-crt-locale-l1-1-0.dll
```

produces the following output:

```
Original import list:
    KERNEL32.dll
    MSVCP140.dll
    VCRUNTIME140.dll
    VCRUNTIME140_1.dll
    api-ms-win-crt-runtime-l1-1-0.dll
    api-ms-win-crt-heap-l1-1-0.dll
    api-ms-win-crt-convert-l1-1-0.dll
    api-ms-win-crt-environment-l1-1-0.dll
    api-ms-win-crt-stdio-l1-1-0.dll
    api-ms-win-crt-filesystem-l1-1-0.dll
    api-ms-win-crt-math-l1-1-0.dll
    api-ms-win-crt-locale-l1-1-0.dll

Reordered import list:
    VCRUNTIME140.dll
    api-ms-win-crt-locale-l1-1-0.dll
    KERNEL32.dll
    MSVCP140.dll
    VCRUNTIME140_1.dll
    api-ms-win-crt-runtime-l1-1-0.dll
    api-ms-win-crt-heap-l1-1-0.dll
    api-ms-win-crt-convert-l1-1-0.dll
    api-ms-win-crt-environment-l1-1-0.dll
    api-ms-win-crt-stdio-l1-1-0.dll
    api-ms-win-crt-filesystem-l1-1-0.dll
    api-ms-win-crt-math-l1-1-0.dll
```

If you list the imports again, you'll find them in the order shown above under "Reordered import list".

### CMake usage

PEachy works on any Windows system, but if you're integrating this in a CMake project, the following snippet might be useful:

```cmake
add_custom_command(TARGET MyTarget POST_BUILD
    COMMAND
        "${CMAKE_SOURCE_DIR}/peachy.exe"
        escalate
        $<TARGET_FILE:MyTarget>
        $<$<CONFIG:debug>:mimalloc-debug.dll>
        $<$<NOT:$<CONFIG:debug>>:mimalloc.dll>
    COMMENT
        "Prioritize mi-malloc dlls in import order"
)
```

The example above escalates `mimalloc-debug.dll` or `mimalloc.dll` to the front of the import list for a given target `MyTarget` for debug and non-debug builds respectively.
As a `POST_BUILD` custom command, this runs each time `MyTarget` is built.
