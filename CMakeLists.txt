cmake_minimum_required(VERSION 3.13)
project(log4cxx_10_to_11)
include(CTest)

set(Boost_USE_STATIC_LIBS  ON)  # only find static libs
find_package(Boost COMPONENTS wave filesystem program_options unit_test_framework REQUIRED )
find_package(log4cxx 0.11 REQUIRED )
if(WIN32 AND NOT BUILD_SHARED_LIB)
  foreach(flag_var
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif()
  endforeach()
endif()
add_subdirectory(util)

add_executable(log4cxx_10_to_11
  log4cxx_10_to_11.cpp 
)
target_compile_definitions(log4cxx_10_to_11 PRIVATE $<TARGET_PROPERTY:log4cxx,INTERFACE_COMPILE_DEFINITIONS> BOOST_WAVE_STATIC_LINK)
target_include_directories(log4cxx_10_to_11 PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(log4cxx_10_to_11 PRIVATE Util log4cxx ${Boost_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

