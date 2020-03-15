cmake_minimum_required(VERSION 3.13)
project(log4cxx_10_to_11)
include(CTest)

# BOOST
set(Boost_USE_STATIC_LIBS  ON)  # only find static libs
find_package(Boost COMPONENTS wave unit_test_framework REQUIRED )
find_package(log4cxx CONFIG REQUIRED )

add_library(log4cxx_10_to_11 STATIC
  CppFile.cpp
)
target_compile_definitions(log4cxx_10_to_11 PRIVATE BOOST_WAVE_STATIC_LINK)
target_include_directories(log4cxx_10_to_11 PRIVATE ${Boost_INCLUDE_DIRS})

add_executable(log4cxx_10_to_11_tests
  log4cxx_10_to_11_tests.cpp 
)
target_compile_definitions(log4cxx_10_to_11_tests PRIVATE BOOST_WAVE_STATIC_LINK)
target_include_directories(log4cxx_10_to_11_tests PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(log4cxx_10_to_11_tests PRIVATE log4cxx_10_to_11 log4cxx ${Boost_LIBRARIES})

add_test(NAME log4cxx_10_to_11_tests
    COMMAND log4cxx_10_to_11_tests  --report_level=no --log_level=test_suite
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/resources
)

