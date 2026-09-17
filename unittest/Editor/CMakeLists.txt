# set(TEST_SOURCES Recognize.cpp)

foreach(test_file IN LISTS TEST_SOURCES)
    get_filename_component(test_name ${test_file} NAME_WE)
    add_unit_test(${test_name} ${test_file})
endforeach()
