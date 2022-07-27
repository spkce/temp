
#include "EventId.h"
#include "ctime.h"

namespace Event
{

CEventID* CEventID::instance()
{
	static CEventID inst;
	return &inst;
}

CEventID::CEventID()
:m_time(0)
,m_times(0)
{

}

CEventID::~CEventID()
{

}

EventId CEventID::getId()
{
	unsigned long long realTime = Infra::CTime::getRealTimeSecond();
	unsigned long long systemTime = Infra::CTime::getSystemTimeSecond();
	unsigned long long temp = (realTime & 0xffffffff) << 32 | (systemTime & 0xffff) << 16;

	if (temp == m_time)
	{
		m_times++;
	}
	else
	{
		m_times = 0;
	}

	m_time = temp;
	return temp | (m_times & 0xffff);
}

}//Event