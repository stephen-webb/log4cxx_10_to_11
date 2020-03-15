#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>

// Test #ifdef
#ifdef UNDEFINED_ITEM
static int xxxx(UNDEFINED_ITEM);
#endif

// test continuation line
#define SOME_MACRO_FUNCTION_START static void func1() \
{
#define SOME_MACRO_FUNCTION_END  return 0; \
} \

SOME_MACRO_FUNCTION_START
int a = 0;
if (123 == a)
    return false;
else
    return true;
SOME_MACRO_FUNCTION_END

/* Test C style comment
 */
int main(int ac, char** av)
{
    log4cxx::LoggerPtr myLog(log4cxx::Logger::getLogger("main"));
	bool ok = true;
	std::string processName = av[0];
    log4cxx::PropertyConfigurator::configure(processName + ".properties");

    // Log a greeting
    std::string greeting;
    greeting = "---------------------- Welcome to " + processName + " ----------------------";
    greeting =  std::string(greeting.size(),'-') + '\n' + greeting + '\n' + std::string(greeting.size(),'-');
    LOG4CXX_INFO(myLog, "\n\n" << greeting)
    if (2 < ac)
        LOG4CXX_DEBUG(myLog, "av[1]=" << av[1] << " av[2]=" << av[2]) // This needs a ; for 0.11
    else if (1 < ac)
        LOG4CXX_DEBUG(myLog, "av[1]=" << av[1]) // This needs a ; for 0.11
    else
        LOG4CXX_DEBUG(myLog, "missing arg") // This needs a ; for 0.11
    return ok ? 0 : 1;
}
