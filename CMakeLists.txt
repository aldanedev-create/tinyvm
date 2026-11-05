cmake_minimum_required(VERSION 3.16)

project(tinyvm C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(tinyvm_core STATIC
    src/core/value.c
    src/core/stack.c
    src/core/vm.c
)

target_include_directories(tinyvm_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
)

add_executable(tinyvm_tests tests/test_vm.c)
target_link_libraries(tinyvm_tests PRIVATE tinyvm_core)

enable_testing()
add_test(NAME tinyvm_tests COMMAND tinyvm_tests)