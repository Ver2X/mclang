cmake_minimum_required(VERSION 3.24.2)
project(mclang LANGUAGES CXX)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
SET(CMAKE_CXX_COMPILER clang++)

set(CMAKE_CXX_STANDARD 23)
list(APPEND CMAKE_CXX_FLAGS "-fno-common -g")

if (ENABLE_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fsanitize=address -fno-omit-frame-pointer -g")
endif()
message("C++ compiler flags: ${CMAKE_CXX_FLAGS}")
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Set Build Type to Debug")

include_directories(include)
add_library(libs SHARED 
    lib/front/parse.cpp
    lib/front/strings.cpp
    lib/front/tokenize.cpp
    lib/front/type.cpp

    lib/ir/BasicBlock.cpp 
    lib/ir/FunctionIR.cpp 
    lib/ir/IRGen.cpp 
    lib/ir/Instruction.cpp
    lib/ir/SymbolTable.cpp
    lib/ir/Value.cpp
    lib/ir/IRBuilder.cpp
    lib/ir/Module.cpp
    lib/ir/IRGenStatement.cpp
    lib/ir/IRGenAddr.cpp
    lib/ir/IRGenExpression.cpp
    lib/ir/StructTypeTag.cpp

    lib/utils/Rope.cpp

    lib/analysis/Dominance.cpp
    lib/analysis/DominatorTree.cpp

    lib/transform/Mem2reg.cpp
    lib/transform/DCE.cpp

    lib/driver/codegen.cpp
)

add_executable(mclang lib/driver/main.cpp)
target_link_libraries(mclang libs)