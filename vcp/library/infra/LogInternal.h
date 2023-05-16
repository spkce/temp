#ifndef __LOG_INTERNAL_H__
#define __LOG_INTERNAL_H__

namespace Infra
{

void InteralPrint(const char* file, int line, const char* func, const char* fmt, ...) __attribute__((format(printf, 4, 5)));

#ifdef INFRA_LOG
#define InfraTrace(fmt, ...) InteralPrint(__FILE__, __LINE__, __FUNCTION__, (fmt), ## __VA_ARGS__)
#else
#define InfraTrace(fmt, ...)
#endif

} //Infra

#endif //__LOG_INTERNAL_H__
