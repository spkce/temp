
#include "stdio.h"
#include "ctime.h"
#include <sys/select.h>
#include <errno.h>
#include <string.h>

namespace Infra
{

CDate::CDate()
{
	timespec time;
	clock_gettime(CLOCK_REALTIME, &time);
	m_timeStamp = time.tv_sec;
}

CDate::CDate(time_t timeStamp)
{
	m_timeStamp = timeStamp;
}

CDate::CDate(CDate &date)
{
	m_timeStamp = date.m_timeStamp;
}

CDate::CDate(PTDate p)
{
	m_timeStamp = convertDate(p);
}

CDate::CDate(const TDate & tDate)
{
	m_timeStamp = convertDate(tDate);
}

CDate::~CDate()
{

}

time_t CDate::convertDate(PTDate p)
{
	if (p != NULL)
	{
		struct tm stDate = {0};
		stDate.tm_year = p->year - 1900;
		stDate.tm_mon =  p->month - 1;
		stDate.tm_mday = p->day;
		stDate.tm_hour = p->hour;
		stDate.tm_min = p->minute;
		stDate.tm_sec = p->second;

		return mktime(&stDate);
	}
	else
	{
		return 0;
	}
}

time_t CDate::convertDate(const TDate & tDate)
{
	struct tm stDate = {0};
	stDate.tm_year = tDate.year - 1900;
	stDate.tm_mon = tDate.month - 1;
	stDate.tm_mday = tDate.day;
	stDate.tm_hour = tDate.hour;
	stDate.tm_min = tDate.minute;
	stDate.tm_sec = tDate.second;

	return mktime(&stDate);
}

CDate& CDate::operator=(const CDate& o)
{
	m_timeStamp = o.m_timeStamp;
	return *this;
}

bool CDate::operator==(const CDate& o)
{
	return m_timeStamp == o.m_timeStamp;
}

bool CDate::operator>(const CDate& o)
{
	return m_timeStamp > o.m_timeStamp;
}

bool CDate::operator<(const CDate& o)
{
	return m_timeStamp < o.m_timeStamp;
}

const char* CDate::str()
{
	static char strDate[sizeof("YYYY-MM-DD HH:MM:SS")];

	struct tm stDate;
	if (localtime_r(&m_timeStamp, &stDate) != NULL)
	{
		snprintf(strDate, sizeof(strDate),"%04d-%02d-%02d %02d:%02d:%02d", stDate.tm_year + 1900, stDate.tm_mon + 1, stDate.tm_mday, stDate.tm_hour, stDate.tm_min, stDate.tm_sec);
	}
	else
	{
		strncpy(strDate, "1970-01-01 00:00:00", sizeof(strDate));
	}

	return (const char*)&strDate;
}

time_t CDate::get()
{
	return m_timeStamp;
}

bool CDate::get(PTDate p)
{
	if (p == NULL)
	{
		return false;
	}

	struct tm stDate;
	if (localtime_r(&m_timeStamp, &stDate) != NULL)
	{
		p->year = stDate.tm_year + 1900;
		p->month = stDate.tm_mon + 1;
		p->day = stDate.tm_mday;
		p->hour = stDate.tm_hour;
		p->minute = stDate.tm_min;
		p->second = stDate.tm_sec;
		return true;
	}

	return false;
}

bool CDate::get(TDate & t)
{
	struct tm stDate;
	if (localtime_r(&m_timeStamp, &stDate) != NULL)
	{
		t.year = stDate.tm_year + 1900;
		t.month = stDate.tm_mon + 1;
		t.day = stDate.tm_mday;
		t.hour = stDate.tm_hour;
		t.minute = stDate.tm_min;
		t.second = stDate.tm_sec;
		return true;
	}

	return false;
}

unsigned long long CTime::getRealTimeSecond()
{
	timespec time;
	clock_gettime(CLOCK_REALTIME, &time);
	return time.tv_sec;
}


unsigned long long CTime::getRealTimeMSecond()
{
	timespec time;
	clock_gettime(CLOCK_REALTIME, &time);
	return time.tv_nsec /1000000 + time.tv_sec* 1000;
}

unsigned long long CTime::getSystemTimeSecond()
{
	timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	return time.tv_sec;
}

unsigned long long CTime::getSystemTimeMSecond()
{
	timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	return time.tv_nsec /1000000 + time.tv_sec* 1000;
}

unsigned long long CTime::getProcessTimeSecond()
{
	timespec time;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time); 
	return time.tv_sec;
}

unsigned long long CTime::getProcessTimeMSecond()
{
	timespec time;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
	return time.tv_nsec /1000000 + time.tv_sec* 1000;
}

unsigned long long CTime::getThreadTimeSecond()
{
	timespec time;
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &time);
	return time.tv_sec;
}

unsigned long long CTime::getThreadTimeMSecond()
{
	timespec time;
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &time);
	return time.tv_nsec /1000000 + time.tv_sec* 1000;
}

void CTime::covertRealTime(unsigned int ms, timespec *tp)
{
	unsigned long long _ms = getRealTimeMSecond() + ms;

	tp->tv_sec = _ms/1000;
	tp->tv_nsec = (_ms%1000)*1000;
}

void CTime::delay_ms(unsigned int ms)
{
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = ms*1000;
	int err;
	do
	{
		err = select(0,NULL,NULL,NULL,&tv);
	}while (err<0 && errno==EINTR);
}

}//Infra
