# cmake -DCMAKE_PREFIX_PATH=~/llvm-mos .. from the build directory
cmake_minimum_required(VERSION 3.18)
set(LLVM_MOS_PLATFORM cx16)
find_package(llvm-mos-sdk REQUIRED)

option(USE_SOFT_FLOAT "Use the bsoftfloat library" ON)

project(derpicalc)
set(derpicalc_sources dc.c cell.c expr.c expr_at.c input.c
    keyboard.c screen.c ui.c ui_edit.c util.c)
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
