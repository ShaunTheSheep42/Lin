list(APPEND CMAKE_MODULE_PATH "${EXTERNAL_LIBS_DIR}/catch2")

include(Catch)

include(AddUnitTest)
add_subdirectory(Basic)
