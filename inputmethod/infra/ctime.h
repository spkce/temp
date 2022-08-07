#ifndef __TIME_H__
#define __TIME_H__

#include <time.h>
namespace Infra
{

typedef struct tagDate
{
	tagDate() {}
	tagDate(unsigned int year, unsigned int month, unsigned int day, unsigned int hour, unsigned int minute, unsigned int second)
	{
		this->year = year;
		this->month = month;
		this->day = day;
		this->hour = hour;
		this->minute = minute;
		this->second = second;
	}
	unsigned int year; //[1970,2038]
	unsigned int month; //[1,12]
	unsigned int day; //[1, 31]
	unsigned int hour; //[0, 23]
	unsigned int minute;//[0, 23]
	unsigned int second;//[0, 23]
}TDate, *PTDate;

class CDate
{
public:
	CDate();
	CDate(time_t timeStamp);
	CDate(CDate &data);
	CDate(PTDate p);
	CDate(const TDate & tDate);
	virtual ~CDate();

	static time_t convertDate(PTDate p);
	static time_t convertDate(const TDate & tDate);
public:

	CDate& operator=(const CDate& o);
	bool operator==(const CDate& o);
	bool operator>(const CDate& o);
	bool operator<(const CDate& o);

	const char* str();
	time_t get();
	bool get(PTDate p);
	bool get(TDate & t);
private:

	time_t m_timeStamp;
};

class CTime
{
public:
	CTime();
	CTime(CDate &t);
	virtual ~CTime();
	//获取当前unix时间戳, 单位:秒
	static unsigned long long getRealTimeSecond();
	//获取当前unix时间戳, 单位:毫秒
	static unsigned long long getRealTimeMSecond();
	//获取系统运行时间, 单位:秒
	static unsigned long long getSystemTimeSecond();
	//获取系统运行时间, 单位:纳秒
	static unsigned long long getSystemTimeMSecond();
	//获取进程运行时间, 单位:秒
	static unsigned long long getProcessTimeSecond();
	//获取进程运行时间, 单位:纳秒
	static unsigned long long getProcessTimeMSecond();
	//获取线程运行时间, 单位:秒
	static unsigned long long getThreadTimeSecond();
	//获取线程运行时间, 单位:纳秒
	static unsigned long long getThreadTimeMSecond();

	//将时间转换成系统实时时间，当前系统实时时间增加ms时间 
	static void covertRealTime(unsigned int ms, timespec *tp);

	static void delay_ms(unsigned int ms);
private:
};

}//Infra

#endif //__TIME_H__
