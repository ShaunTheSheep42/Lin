cmake_minimum_required(VERSION 3.28...3.30)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS NO)

project(
    Lin
    VERSION 1.0.0
    DESCRIPTION "A pseudo-TUI editor for keyboard-driven, non-linear editing"
)

# Clang(Clang++) version >= 22.0
set(CLANG_VERSION "22.0")
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS ${CLANG_VERSION})
    message(STATUS "C++ compiler version: ${CMAKE_CXX_COMPILER_VERSION}")
    message(
        FATAL_ERROR
        "Clang(Clang++) version must be at least ${CLANG_VERSION}"
    )
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_subdirectory(external)
find_package(Threads)

add_subdirectory(lib)

option(ENABLE_TEST "Enable building tests" OFF)
if(ENABLE_TEST)
    enable_testing()
    add_subdirectory(${EXTERNAL_LIBS_DIR}/catch2)
    add_subdirectory(unittest)
endif()

# This line must go below project(..)
include(GNUInstallDirs)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/config/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/lin
)
