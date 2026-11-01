cmake_minimum_required(VERSION 3.16)

project(tinyvm C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(tinyvm_core STATIC
  src/core/value.c
  src/core/stack.c

)

target_include_directories(tinyvm_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
)

add_executable(tinyvm_smoke src/core/main.c)
target_link_libraries(tinyvm_smoke PRIVATE  tinyvm_core)
