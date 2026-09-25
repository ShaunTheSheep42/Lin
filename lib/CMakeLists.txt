set(LIN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LIN_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)

include(AddLib)

add_subdirectory(Basic)
add_subdirectory(App)

add_executable(lin main.cpp)
target_link_libraries(lin PUBLIC LinApp)

set_target_properties(lin PROPERTIES BUILD_RPATH "${CMAKE_BINARY_DIR}/lib")

install(TARGETS lin RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
