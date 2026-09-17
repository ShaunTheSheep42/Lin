# Add unit test
function(add_unit_test name file)
    set(test_name "${name}Tests")

    add_executable(${test_name} ${file})

    target_link_libraries(${test_name} PRIVATE LinCore Qt6::Test)

    add_test(NAME ${test_name} COMMAND ${test_name})

    set_tests_properties(${test_name} PROPERTIES LABELS "unittest")
endfunction()
