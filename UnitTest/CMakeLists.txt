file(GLOB_RECURSE unit_test_src_c *.c)
file(GLOB_RECURSE unit_test_src_cpp *.cpp)

add_executable(UNIT_TEST main.cpp ${unit_test_src_c} ${unit_test_src_cpp})

target_include_directories(UNIT_TEST PUBLIC ./TestMock)
target_include_directories(UNIT_TEST PUBLIC ./TestSrc)
target_include_directories(UNIT_TEST PUBLIC ./TestStub)

target_link_libraries(UNIT_TEST PUBLIC APP)
target_link_libraries(UNIT_TEST PUBLIC MODBUS)
target_link_libraries(UNIT_TEST PUBLIC DING_OS)
target_link_libraries(UNIT_TEST PUBLIC OS_AL_LIB)

target_include_directories(UNIT_TEST PUBLIC ./CppUTest)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if (MSVC)
        target_link_directories(UNIT_TEST PUBLIC ./CppUTest/MSVC)
        target_link_libraries(UNIT_TEST PUBLIC CppUTestd)
    endif ()
endif ()


if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_directories(UNIT_TEST PUBLIC ./CppUTest/GCC)
    target_link_libraries(UNIT_TEST PUBLIC libCppUTest.a libCppUTestExt.a)
endif ()


add_custom_command(TARGET UNIT_TEST POST_BUILD COMMAND UNIT_TEST)







