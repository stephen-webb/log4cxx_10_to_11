add_executable(log4cxx_10_to_11_tests
  CppFileTests.cpp 
)
target_compile_definitions(log4cxx_10_to_11_tests PRIVATE $<TARGET_PROPERTY:log4cxx,INTERFACE_COMPILE_DEFINITIONS> ${Boost_COMPILE_DEFINITIONS} BOOST_WAVE_STATIC_LINK)
target_include_directories(log4cxx_10_to_11_tests PRIVATE .. $<TARGET_PROPERTY:log4cxx,INTERFACE_INCLUDE_DIRECTORIES> ${Boost_INCLUDE_DIRS})
target_link_libraries(log4cxx_10_to_11_tests PRIVATE Util log4cxx ${Boost_LIBRARIES})

add_test(NAME log4cxx_10_to_11_tests
    COMMAND log4cxx_10_to_11_tests  --report_level=no --log_level=test_suite
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/resources
)
set(path_var
  $<TARGET_FILE_DIR:log4cxx_10_to_11_tests>
  ${LOG4CXX_FILE_DIR}
)
list(REMOVE_DUPLICATES path_var)
if(WIN32)
  set_target_properties(log4cxx_10_to_11_tests PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/resources
  )
  set_target_properties(log4cxx_10_to_11_tests PROPERTIES
    VS_DEBUGGER_COMMAND_ARGUMENTS "--run_test=*/*/* --log_level=error --report_level=no"
  )
  set_target_properties(log4cxx_10_to_11_tests PROPERTIES
    VS_DEBUGGER_ENVIRONMENT "PATH=$<JOIN:${path_var},\;>"
  )
  set_tests_properties(log4cxx_10_to_11_tests PROPERTIES
    ENVIRONMENT "PATH=$<JOIN:${path_var},\;>"
  )
else()
  set_tests_properties(log4cxx_10_to_11_tests PROPERTIES
    ENVIRONMENT "LD_LIBRARY_PATH=$<JOIN:${path_var},:>"
  )
endif()
