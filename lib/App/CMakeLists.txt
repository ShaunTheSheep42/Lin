add_lin_library(
    App
    STATIC
    Lin.cpp

    MODULE
    Lin.cppm

    LINK
    Support

    INCLUDE
    ${EXTERNAL_LIBS_DIR}/cli11
)
