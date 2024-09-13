cmake_minimum_required(VERSION 3.27)
project(compiler)

set(CMAKE_CXX_STANDARD 23)

set(SOURCES
    Analysis/Analysis.cpp
    Analysis/AvailableExpressions.cpp
    Analysis/BlockSort.cpp
    Analysis/Constants.cpp
    Analysis/DominanceFrontiers.cpp
    Analysis/DominatorTree.cpp
    Analysis/FlowGraph.cpp
    Analysis/InterferenceGraph.cpp
    Analysis/LiveVariables.cpp
    Analysis/Loops.cpp
    Analysis/ReachingDefs.cpp
    Analysis/UseDefs.cpp
    Back/AsmParser.cpp
    Back/AsmTokenizer.cpp
    Back/CodeGenerator.cpp
    Back/RegisterAllocator.cpp
    Front/EnvironmentGenerator.cpp
    Front/ExportInfo.cpp
    Front/HllParser.cpp
    Front/HllTokenizer.cpp
    Front/IRGenerator.cpp
    Front/Node.cpp
    Front/Procedure.cpp
    Front/Program.cpp
    Front/ProgramGenerator.cpp
    Front/Scope.cpp
    Front/Type.cpp
    Front/Types.cpp
    Input/Parser.cpp
    Input/Tokenizer.cpp
    IR/Data.cpp
    IR/Entry.cpp
    IR/EntryList.cpp
    IR/Procedure.cpp
    IR/Program.cpp
    Middle/ErrorCheck.cpp
    Middle/Optimizer.cpp
    Transform/CommonSubexpressionElimination.cpp
    Transform/ConstantProp.cpp
    Transform/CopyProp.cpp
    Transform/DeadCodeElimination.cpp
    Transform/LiveRangeRenaming.cpp
    Transform/LoopInvariantCodeMotion.cpp
    Transform/SSA.cpp
    Transform/ThreadJumps.cpp
    Util/Log.cpp
    VM/AddressSpace.cpp
    VM/GarbageCollector.cpp
    VM/Heap.cpp
    VM/Instruction.cpp
    VM/Interp.cpp
    VM/OrcFile.cpp
    VM/Program.cpp
    Assembler.cpp
    Compiler.cpp
    Linker.cpp
    Main.cpp
)

include_directories(${CMAKE_SOURCE_DIR})
add_executable(compiler ${SOURCES})