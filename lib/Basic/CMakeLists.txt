add_lin_library(
    Basic

    STATIC
    Block.cpp
    Char.cpp
    Text.cpp
    Canvas.cpp
    SpatialIndex.cpp
    Buffer.cpp
    History.cpp
    View.cpp

    MODULE
    Geometry.cppm
    Char.cppm
    String.cppm
    Block.cppm
    Text.cppm
    Canvas.cppm
    SpatialIndex.cppm
    Buffer.cppm
    History.cppm
    View.cppm

    LINK
    Debug
    Support
)

target_include_directories(LinBasic PUBLIC ${EXTERNAL_LIBS_DIR}/glaze)
