cmake_minimum_required(VERSION 3.13)
project(log4cxx_10_to_11)
include(CTest)

set(Boost_USE_STATIC_LIBS  ON)  # only find static libs
find_package(Boost COMPONENTS wave filesystem program_options unit_test_framework REQUIRED )
find_package(log4cxx CONFIG REQUIRED )

add_subdirectory(util)

add_executable(log4cxx_10_to_11
  log4cxx_10_to_11.cpp 
)
target_compile_definitions(log4cxx_10_to_11 PRIVATE BOOST_WAVE_STATIC_LINK)
target_include_directories(log4cxx_10_to_11 PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(log4cxx_10_to_11 PRIVATE Util log4cxx ${Boost_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

