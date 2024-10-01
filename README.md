# Will Compile C for Food

## Description

My adventure in compilers land.
Implementing a C compiler by following the great book [Writing a C Compiler](https://norasandler.com/book/) from Nora
Sandler.

This is the type of programs that it can compile.

```c++
int main(void)
{
    return 1 + 2;
}
```

At the moment, the compiler will only generate binaries for macOS x64.

## Installation

The best way to get all compile it is by using [vcpkg](https://vcpkg.io/en/).

Create a file named CMakeUserPresets.json with the following content.
Don't forget to update VCPKG_ROOT with the location of your vcpkg installation.

```json
{
  "version": 4,
  "cmakeMinimumRequired": {
    "major": 3,
    "minor": 29,
    "patch": 0
  },
  "configurePresets": [
    {
      "name": "dev",
      "binaryDir": "${sourceDir}/build/dev",
      "inherits": [
        "vcpkg",
        "debug-build"
      ],
      "generator": "Ninja",
      "cacheVariables": {
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
      },
      "environment": {
        "VCPKG_ROOT": "Path to vcpkg installation directory"
      }
    }
  ]
}
```

Then just run to build the compiler and test suite.

```
make --preset dev
ninja
```

## Compile something

The compiler acts as a "normal" compiler, it just needs to receive the file to compile as
a parameter, it will compile it and output the binary at the same location of the source file.
The binary will have the same name as the source file, minus the .c extension.

The following commands, compile the source code named sample.c, run the executable and
prints the return code of the execution.
For example, if sample.c contains the code shown on the example of this page, then the
output of the echo will be 3.

```
./build/dev/wccff sample.c
./sample
echo $?
```

Note: When running this on an Arm Macbook, this needs to be executed inside Rosetta.
Run this command to start a new x64 console ` arch -x86_64 zsh`

There are a few flags that stop the compilation at certain points.

* --lex, Runs the Lexer
* --parse, Runs the Lexer and Parser
* --tacky, Run the Lexer, Parser and Tacky
* --codegen, Run the Lexer, Parser, Tacky and Code Generation