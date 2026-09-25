# Add lin unit test
function(add_lin_unittest name)
    cmake_parse_arguments(ARG "" "" "LINK" ${ARGN})

    set(test_name "LinUnitTest${name}")

    add_executable(${test_name} ${ARG_UNPARSED_ARGUMENTS})

    target_link_libraries(
        ${test_name}
        PRIVATE Catch2::Catch2WithMain Threads::Threads ${ARG_LINK}
    )

    catch_discover_tests(${test_name} PROPERTIES LABELS "unittest")
endfunction()
