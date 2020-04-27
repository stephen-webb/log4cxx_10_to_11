cmake_minimum_required(VERSION 3.13)
project(log4cxx_10_to_11)
include(CTest)
if(EXISTS ${CMAKE_BINARY_DIR}/conan_paths.cmake)
  include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
endif()

set(Boost_USE_STATIC_LIBS  ON)
find_package(Boost COMPONENTS wave filesystem program_options unit_test_framework REQUIRED )
message("-- Found Boost ${Boost_INCLUDE_DIR}")
if(WIN32)
  get_target_property(Boost_COMPILE_DEFINITIONS
    Boost::disable_autolinking INTERFACE_COMPILE_DEFINITIONS
  )
endif()
find_package(log4cxx 0.11 REQUIRED )
get_target_property(LOG4CXX_DLL_PATH log4cxx IMPORTED_LOCATION_RELEASE)
get_filename_component(LOG4CXX_FILE_DIR "${LOG4CXX_DLL_PATH}" PATH)
message("-- Found log4cxx ${LOG4CXX_FILE_DIR}")

# Add support for linking statically
option(BUILD_SHARED_LIBS "Use dynamically linked runtime" ON)
if(WIN32 AND NOT BUILD_SHARED_LIBS)
  foreach(flag_var
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif()
  endforeach()
endif()

# Building
add_subdirectory(util)
add_executable(log4cxx_10_to_11
  log4cxx_10_to_11.cpp 
)
target_compile_definitions(log4cxx_10_to_11 PRIVATE $<TARGET_PROPERTY:log4cxx,INTERFACE_COMPILE_DEFINITIONS> ${Boost_COMPILE_DEFINITIONS} BOOST_WAVE_STATIC_LINK)
target_include_directories(log4cxx_10_to_11 PRIVATE $<TARGET_PROPERTY:log4cxx,INTERFACE_INCLUDE_DIRECTORIES> ${Boost_INCLUDE_DIRS})
target_link_libraries(log4cxx_10_to_11 PRIVATE Util log4cxx ${Boost_LIBRARIES})

# Testing
if(BUILD_TESTING)
  add_subdirectory(test)
endif()

# Installation
include(GNUInstallDirs)
install(CODE [[
  file(GET_RUNTIME_DEPENDENCIES
    RESOLVED_DEPENDENCIES_VAR log4cxx_10_to_11_dependencies
    UNRESOLVED_DEPENDENCIES_VAR log4cxx_10_to_11_unresolved
    POST_EXCLUDE_REGEXES ^/lib ^/usr/lib ^/usr/local/lib
    EXECUTABLES log4cxx_10_to_11
    )
  #message("log4cxx_10_to_11_dependencies=${log4cxx_10_to_11_dependencies}")
  #message("log4cxx_10_to_11_unresoved=${log4cxx_10_to_11_unresolved}")
  file(INSTALL ${log4cxx_10_to_11_dependencies}
    DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
    FOLLOW_SYMLINK_CHAIN
  )
]])
install(TARGETS log4cxx_10_to_11
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

