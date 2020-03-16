
add_library(Util STATIC
  CppFile.cpp
  DirectoryEntryIterator.cpp
)
target_compile_definitions(Util PRIVATE BOOST_WAVE_STATIC_LINK)
target_include_directories(Util PRIVATE ${Boost_INCLUDE_DIRS})
