cmake_minimum_required(VERSION 3.18)

project(llvm_hello_world)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

find_package(LLVM REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH ${LLVM_CMAKE_DIR})


include(AddLLVM)

#TEST
set(tmp1  ${CMAKE_MODULE_PATH})
message(STATUS "MESSAGE: ${tmp1}")
#TEST

include_directories("${LLVM_BINARY_DIR}/include" "${LLVM_INCLUDE_DIR}")
add_definitions(${LLVM_DEFINITIONS})

if (NOT ${LLVM_ENABLE_RTTI})
    # For non-MSVC compilers
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

set(LLVM_LINK_COMPONENTS Support Core)
LINK_LIBRARIES("/home/rain/llvm_project_1706/llvm_install_dir/lib/")

# 以前的版本
# add_executable(${PROJECT_NAME} main.cc)
# 修改后的版本
add_llvm_executable(${PROJECT_NAME} main.cc parser.cc Lexer.cpp ast.h CodeGen.h CodeGen.cpp)


#target_link_libraries(llvm_hello_world Support Core)