# Add lin library
function(add_lin_library name)
    cmake_parse_arguments(ARG "STATIC;SHARED" "" "MODULE;INCLUDE;LINK" ${ARGN})

    if(ARG_STATIC)
        set(LIB_FORMAT STATIC)
    else()
        set(LIB_FORMAT SHARED)
    endif()

    set(library_name "Lin${name}")

    add_library(${library_name} ${LIB_FORMAT} ${ARG_UNPARSED_ARGUMENTS})
    add_library(${name} ALIAS ${library_name})

    target_sources(
        ${library_name}
        PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES ${ARG_MODULE}
    )

    target_include_directories(${library_name} PRIVATE ${ARG_INCLUDE})

    target_link_libraries(
        ${library_name}
        PRIVATE
            # Due to the constraint of Options consistency in Consistency Requirements
            # See https://clang.llvm.org/docs/StandardCPlusPlusModules.html#options-consistency
            Threads::Threads
            ${ARG_LINK}
    )

    install(TARGETS ${library_name} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
endfunction()
