add_lin_library(
    Basic

    STATIC
    Char.cpp
    String.cpp
    # Block.cpp

    LINK
    Support

    MODULE
    Geometry.cppm
    Char.cppm
    String.cppm
    # Block.cppm
)
