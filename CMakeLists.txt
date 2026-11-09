cmake_minimum_required(VERSION 3.21)

project(tengrad-c LANGUAGES C)
if(PROJECT_IS_TOP_LEVEL AND NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

add_library(tengrad_sanitizers INTERFACE)

option(TENGRAD_SANITIZE "Enable Address and Undefined Behavior sanitizers" OFF)
if(TENGRAD_SANITIZE)
    # 3. Add compiler and linker flags
    target_compile_options(tengrad_sanitizers INTERFACE -fsanitize=address,undefined -g -O1)
    target_link_options(tengrad_sanitizers INTERFACE -fsanitize=address,undefined)
    message(STATUS "Sanitizers are ENABLED")
endif()

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(tengrad_warnings INTERFACE)
target_compile_options(tengrad_warnings INTERFACE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
option(TENGRAD_WERROR "Treat compiler warnings as errors" OFF)

if(TENGRAD_WERROR)
    target_compile_options(tengrad_warnings INTERFACE -Werror)
endif()

add_library(tengrad STATIC src/version.c)
target_include_directories(tengrad PUBLIC include)
target_link_libraries(tengrad PRIVATE tengrad_warnings tengrad_sanitizers)


add_executable(smoke_version examples/smoke/main.c)
target_link_libraries(smoke_version PRIVATE tengrad tengrad_warnings tengrad_sanitizers)

enable_testing()
add_subdirectory(tests)
