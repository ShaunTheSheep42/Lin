add_lin_library(
    Basic

    STATIC
    Char.cpp
    String.cpp
    # Block.cpp

    MODULE
    Geometry.cppm
    Char.cppm
    String.cppm
    # Block.cppm

    LINK
    Debug
    Support
)
