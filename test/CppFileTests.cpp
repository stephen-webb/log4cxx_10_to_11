#define BOOST_TEST_MODULE log4cxx_10_to_111 test
#define BOOST_TEST_NO_MAIN
#define BOOST_TEST_ALTERNATIVE_INIT_API
#include <boost/test/unit_test.hpp>
#include <log4cxx/propertyconfigurator.h>
#include "util/CppFile.h"

BOOST_AUTO_TEST_CASE( fixup_test )
{
    CppFile file;
    BOOST_REQUIRE(file.LoadFile("main_0_10.cpp"));
    BOOST_CHECK(file.IsValid());
    BOOST_CHECK_EQUAL(file.GetFunctionCount("LOG4CXX_INFO"), 1);
    BOOST_CHECK_EQUAL(file.GetFunctionCount("LOG4CXX_DEBUG"), 3);
    size_t macroCount = 0;
    size_t terminateStatementCount = 0;
    size_t insertBraceCount = 0;
    CppFile::FunctionIterator log4cxxMacro(file, "LOG4CXX_");
    for (log4cxxMacro.Start(); !log4cxxMacro.Off(); log4cxxMacro.Forth())
    {
        ++macroCount;
        if (log4cxxMacro.IsBlock())
            log4cxxMacro.InsertBraces(), ++insertBraceCount;
        else if (!log4cxxMacro.HasStatementTerminator())
            log4cxxMacro.AddSemicolon(), ++terminateStatementCount;
    }
    BOOST_CHECK_EQUAL(macroCount, 4);
    BOOST_CHECK_EQUAL(insertBraceCount, 2);
    BOOST_CHECK_EQUAL(terminateStatementCount, 2);
    BOOST_CHECK(file.StoreFile("main_0_11.cpp"));
}

BOOST_AUTO_TEST_CASE( no_change_test )
{
    CppFile file;
    BOOST_REQUIRE(file.LoadFile("main_0_11.cpp"));
    BOOST_CHECK(file.IsValid());
    size_t macroCount = 0;
    size_t terminateStatementCount = 0;
    size_t insertBraceCount = 0;
    CppFile::FunctionIterator log4cxxMacro(file, "LOG4CXX_");
    for (log4cxxMacro.Start(); !log4cxxMacro.Off(); log4cxxMacro.Forth())
    {
        ++macroCount;
        if (log4cxxMacro.IsBlock())
            ++insertBraceCount;
        else if (!log4cxxMacro.HasStatementTerminator())
            ++terminateStatementCount;
    }
    BOOST_CHECK_EQUAL(macroCount, 4);
    BOOST_CHECK_EQUAL(insertBraceCount, 0);
    BOOST_CHECK_EQUAL(terminateStatementCount, 2);
}

bool init_logger()
{
  log4cxx::PropertyConfigurator::configure("log4cxx_10_to_11.properties");
  return true;
}

int main( int argc, char* argv[] )
{
    return ::boost::unit_test::unit_test_main( &init_logger, argc, argv );
}
