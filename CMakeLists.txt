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
    lib/Rope.cpp
    lib/BasicBlock.cpp 
    lib/FunctionIR.cpp 
    lib/IR.cpp 
    lib/IRGen.cpp 
    lib/Instruction.cpp
    lib/SymbolTable.cpp
    lib/Variable.cpp
    lib/codegen.cpp
    lib/parse.cpp
    lib/strings.cpp
    lib/tokenize.cpp
    lib/type.cpp
    lib/IRBuilder.cpp
    lib/Module.cpp
    lib/IRGenStatement.cpp
    lib/IRGenAddr.cpp
    lib/IRGenExpression.cpp
    lib/StructTypeTag.cpp
    lib/analysis/Dominance.cpp
    lib/transform/Mem2reg.cpp
    lib/analysis/DominatorTree.cpp
)

# include_directories(include)
# add_library(IRBuilder STATIC lib/IRBuilder.cpp)
# add_library(Block STATIC lib/Block.cpp)
# add_library(FunctionIR STATIC lib/FunctionIR.cpp)
# add_library(IR STATIC lib/IR.cpp)
# add_library(IRGen STATIC lib/IRGen.cpp)
# add_library(Instruction STATIC lib/Instruction.cpp)
# add_library(SymbolTable STATIC lib/SymbolTable.cpp)
# add_library(Variable STATIC lib/Variable.cpp)
# add_library(codegen STATIC lib/codegen.cpp)
# add_library(parse STATIC lib/parse.cpp)
# add_library(strings STATIC lib/strings.cpp)
# add_library(tokenize STATIC lib/tokenize.cpp)
# add_library(type STATIC lib/type.cpp)
#add_library(myclang STATIC lib/main.cpp)

add_executable(mclang lib/main.cpp)
target_link_libraries(mclang libs)
# target_link_libraries(myclang 
#     codegen
#     IRBuilder
#     Block
#     FunctionIR
#     IR
#     Instruction
#     SymbolTable
#     Variable
    
#     parse
#     strings
#     tokenize
#     type
# )