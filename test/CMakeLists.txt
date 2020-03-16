add_executable(log4cxx_10_to_11_tests
  CppFileTests.cpp 
)
target_compile_definitions(log4cxx_10_to_11_tests PRIVATE BOOST_WAVE_STATIC_LINK)
target_include_directories(log4cxx_10_to_11_tests PRIVATE .. ${Boost_INCLUDE_DIRS})
target_link_libraries(log4cxx_10_to_11_tests PRIVATE Util log4cxx ${Boost_LIBRARIES})

add_test(NAME log4cxx_10_to_11_tests
    COMMAND log4cxx_10_to_11_tests  --report_level=no --log_level=test_suite
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../resources
)
