# AST Matching for Guidelines
## For proper integration, building, and testing
1. clone `llvm-project` and this repository (rename to `hsc`)
2. move `hsc` to `llvm-project/clang-tools-extra/clang-tidy/`
3. add `add_subdirectory(hsc)` to `llvm-project/clang-tools-extra/clang-tidy/CMakeLists.txt`
4. create the following folders
```
mkdir ~/llvm-project/clang-tools-extra/docs/clang-tidy/checks/hsc
mkdir ~/llvm-project/clang-tools-extra/test/clang-tidy/checkers/hsc
mkdir ~/llvm-project/build
```
5. run
```
cmake -G Ninja ../llvm -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" -DCMAKE_BUILD_TYPE=Release`
ninja clang-tidy
```
