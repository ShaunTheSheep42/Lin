include(FetchContent)

set(EXTERNAL_LIBS_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
    CACHE PATH
    "Directory for External libraries"
)

execute_process(
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/CheckAndFetch.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE result
)

if(NOT result EQUAL 0)
    message(FATAL_ERROR "update.py failed with code ${result}")
endif()
