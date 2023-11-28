# cmake -DCMAKE_PREFIX_PATH=~/llvm-mos .. from the build directory
cmake_minimum_required(VERSION 3.18)

option(USE_CC65 "Use the CC65 compiler" OFF)
option(USE_SOFT_FLOAT "Use the bsoftfloat library" OFF)
set(MOS_PLATFORM cx16)

if(USE_CC65)
    set(USE_LLVM_MOS OFF)
else()
    set(USE_LLVM_MOS ON)
endif(USE_CC65)

if ((NOT USE_LLVM_MOS) AND USE_SOFT_FLOAT)
    message(FATAL_ERROR "bsoftfloat can only be used with llvm-mos")
endif()

if (USE_CC65)
    set(CMAKE_TOOLCHAIN_FILE toolchain/cc65-toolchain.cmake)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -t ${MOS_PLATFORM}")
elseif(USE_LLVM_MOS)
    set(LLVM_MOS_PLATFORM ${MOS_PLATFORM})
    find_package(llvm-mos-sdk REQUIRED)
endif(USE_CC65)

project(derpicalc)
    execute_process(
        COMMAND git rev-parse HEAD
        OUTPUT_VARIABLE git_revision
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    add_definitions(-DGIT_REVISION="${git_revision}")

set(derpicalc_sources dc.c cell.c cell_fmt.c cell_mem.c expr.c expr_at.c
    expr_iter.c input.c keyboard.c screen.c ui.c ui_edit.c util.c)
if(USE_SOFT_FLOAT)
    list(APPEND derpicalc_sources dc_math_double.c)
    add_compile_definitions(DC_MATH_DOUBLE)
else()
    list(APPEND derpicalc_sources dc_math_cx16.c)
    add_compile_definitions(DC_MATH_CX16)
endif(USE_SOFT_FLOAT)
add_executable(dc.prg ${derpicalc_sources})
if(USE_SOFT_FLOAT)
    target_link_libraries(dc.prg bsoftfloat)
endif(USE_SOFT_FLOAT)
