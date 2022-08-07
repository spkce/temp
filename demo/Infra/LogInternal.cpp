
#include "stdio.h"
#include <string.h>
#include "execinfo.h"
#include "Log.h"
#include <stdarg.h>
#include "thread.h"
#include "LogInternal.h"

namespace Infra
{

void InteralPrint(const char* file, int line, const char* func, const char* fmt, ...)
{
	char buffer[8192];
	buffer[8191] = 0;
	int n = snprintf(buffer, sizeof(buffer) - 1, "%s:%d %s ", (file), (line), (func));
	strncpy(buffer + n, (fmt), sizeof(buffer) - 1 - n);
	va_list args;
	va_start(args, (fmt));
	fprintf(stdout, "\033[%dm", 35);
	vfprintf(stdout, buffer, args);
	fprintf(stdout, "\033[0m");
	va_end(args);
}

//static std::string _CutParenthesesNTail(std::string&& prettyFuncon)
//{
//    auto pos = prettyFuncon.find('(');
//    if(pos!=std::string::npos)
//        prettyFuncon.erase(prettyFuncon.begin()+pos, prettyFuncon.end());
//
//    return std::move(prettyFuncon);
//}
//#define __STR_FUNCTION__ _CutParenthesesNTail(std::string(__PRETTY_FUNCTION__))

} //Infra
