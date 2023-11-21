# cmake -DCMAKE_PREFIX_PATH=~/llvm-mos .. from the build directory
cmake_minimum_required(VERSION 3.18)
set(LLVM_MOS_PLATFORM cx16)
find_package(llvm-mos-sdk REQUIRED)

project(derpicalc)
add_executable(dc.prg dc.c dc_math.c cell.c expr.c expr_at.c input.c
               keyboard.c screen.c ui.c ui_edit.c util.c)
