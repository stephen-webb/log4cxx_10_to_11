
add_library(Util STATIC
  CppFile.cpp
  DirectoryEntryIterator.cpp
)
target_compile_definitions(Util PRIVATE $<TARGET_PROPERTY:log4cxx,INTERFACE_COMPILE_DEFINITIONS> BOOST_WAVE_STATIC_LINK)
target_include_directories(Util PRIVATE ${Boost_INCLUDE_DIRS})
