cmake_minimum_required(VERSION 3.14)

# Find out if we're part of a bigger project
set(MASTER_PROJECT OFF)
if (NOT DEFINED PROJECT_NAME)
    set(MASTER_PROJECT ON)
endif()

project(httpserver VERSION 0.1 LANGUAGES CXX)

include(FindPkgConfig)

option(HTTPSERVER_ENABLE_INSTALL "Generate install target" ${MASTER_PROJECT})
option(HTTPSERVER_USE_CLANG_TIDY "Check the code with clang-tidy" ${MASTER_PROJECT})

if (HTTPSERVER_ENABLE_INSTALL)
    include(GNUInstallDirs)
endif()

if (HTTPSERVER_USE_CLANG_TIDY)
    find_program(CMAKE_CXX_CLANG_TIDY NAMES clang-tidy clang-tidy-9 clang-tidy-8 clang-tidy-7)
    if (CMAKE_CXX_CLANG_TIDY_FOUND)
        set(CMAKE_CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY} '-header-filter=.*'")
    endif()
endif()

find_package(Threads REQUIRED)
pkg_check_modules(libmicrohttpd REQUIRED IMPORTED_TARGET libmicrohttpd)

add_library(CompilerWarnings INTERFACE) # Convenience target for enabling compiler warnings
target_compile_options(CompilerWarnings INTERFACE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Wall -Wextra -Wno-unused-parameter>
    $<$<CXX_COMPILER_ID:MSVC>:
    /W4>)

add_library(httpserver)
add_library(aro::httpserver ALIAS httpserver)

target_compile_features(httpserver PUBLIC cxx_std_17)

target_sources(httpserver PRIVATE
    src/InternalRequest.cpp
    src/InternalRequest.h
    src/Request.cpp
    src/RequestHandler.cpp
    src/Response.cpp
    src/Server.cpp
    src/ServerImpl.cpp
    src/ServerImpl.h

    include/http/FunctionTraits.h
    include/http/Request.h
    include/http/RequestHandler.h
    include/http/Response.h
    include/http/Server.h
)

target_include_directories(httpserver PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(httpserver PRIVATE Threads::Threads PkgConfig::libmicrohttpd CompilerWarnings)

