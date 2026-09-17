cmake_minimum_required(VERSION 3.28...3.30)

project(
    Lin
    VERSION 1.0.0
    DESCRIPTION "A pseudo-TUI editor for keyboard-driven, non-linear editing"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core Widgets OpenGLWidgets Test REQUIRED)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

qt_standard_project_setup()

# qt_standard_project_setup does not automatically enable RCC
# so it needs to be enabled manually.
set(CMAKE_AUTORCC ON)

include_directories(lib)

qt_add_library(LinCore
    lib/Lin/Lin.cpp
    lib/Lin/Config.cpp

    lib/Basic/Char.cpp
    lib/Basic/Block.cpp
    lib/Basic/Change.cpp
    lib/Basic/Canvas.cpp

    lib/Edit/Core.h
    lib/Edit/Core.cpp
    lib/Edit/File.cpp
    lib/Edit/Interpreter.cpp
    lib/Edit/Register.cpp
    lib/Edit/Move.cpp
    lib/Edit/Recognize.cpp
    lib/Edit/Undo.cpp
    lib/Edit/CP.cpp
    lib/Edit/Insert.cpp
    lib/Edit/Visual.cpp
    lib/Edit/Lin.cpp
    # lib/Edit/Recognize.cpp

    lib/UI/Core.cpp

    assets/resource.qrc
)

target_link_libraries(
    LinCore
    PUBLIC Qt6::Widgets Qt6::OpenGLWidgets Python3::Python
)

qt_add_executable(lin main.cpp)

target_link_libraries(lin PUBLIC LinCore)

target_compile_options(LinCore PRIVATE -g -O0)
target_compile_options(lin PRIVATE -g -O0)
target_link_options(LinCore PRIVATE -g)
target_link_options(lin PRIVATE -g)

enable_testing()

add_subdirectory(unittest)
