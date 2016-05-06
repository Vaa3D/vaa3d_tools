#!/bin/zsh
ls src/**/*.{cpp,h} vaa3d/*.{cpp,h} test/*.cpp | xargs clang-format -i -style="{ BasedOnStyle: LLVM, Standard: Cpp03, ColumnLimit: 0 }"
