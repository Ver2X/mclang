cmake_minimum_required(VERSION 3.6.0)
project(myclang LANGUAGES CXX)

SET(CMAKE_CXX_COMPILER /usr/bin/clang++)

set(CMAKE_CXX_STANDARD 20)
list(APPEND CMAKE_CXX_FLAGS "-fno-common")
message("C++ compiler flags: ${CMAKE_CXX_FLAGS}")

include_directories(include)
add_library(IRBuilder STATIC lib/IRBuilder.cpp)
add_library(Block STATIC lib/Block.cpp)
add_library(FunctionIR STATIC lib/FunctionIR.cpp)
add_library(IR STATIC lib/IR.cpp)
add_library(IRGen STATIC lib/IRGen.cpp)
add_library(Instruction STATIC lib/Instruction.cpp)
add_library(SymbolTable STATIC lib/SymbolTable.cpp)
add_library(Variable STATIC lib/Variable.cpp)
add_library(codegen STATIC lib/codegen.cpp)
add_library(parse STATIC lib/parse.cpp)
add_library(strings STATIC lib/strings.cpp)
add_library(tokenize STATIC lib/tokenize.cpp)
add_library(type STATIC lib/type.cpp)
#add_library(myclang STATIC lib/main.cpp)

add_executable(myclang lib/main.cpp)
target_link_libraries(myclang 
    codegen
    IRBuilder
    Block
    FunctionIR
    IR
    Instruction
    SymbolTable
    Variable
    
    parse
    strings
    tokenize
    type
)