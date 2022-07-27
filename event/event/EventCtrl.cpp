#include "EventCtrl.h"
#include "Log.h"

namespace Event
{

CEventControl::CEventControl(EventType_t type, unsigned int delay, unsigned int restrain)
:m_type(type)
,m_delay(delay)
,m_restrain(restrain)
,m_saveTime(0)
,m_id(0)
,m_isTrigger(false)
{

}

CEventControl::CEventControl(const CEventControl & o)
:m_type(o.m_type)
,m_delay(o.m_delay)
,m_restrain(o.m_restrain)
,m_saveTime(o.m_saveTime)
,m_id(o.m_id)
,m_isTrigger(o.m_isTrigger)
{

}

CEventControl::~CEventControl()
{

}

bool CEventControl::trigger(unsigned long long runtime)
{
	if (isRestrain(runtime))
	{
		Trace("Event", "event: %d is in restrain\n", m_type);
		return false;
	}

	m_saveTime = runtime;//此时m_saveTime为开始时间

	if (m_isTrigger == false)
	{
		m_id = CEventID::instance()->getId();
		m_isTrigger = true;
		return true;
	}
	else
	{
		return false;
	}
}

void CEventControl::clear(unsigned long long runtime)
{
	m_saveTime = runtime;//此时m_saveTime为结束时间
	m_isTrigger = false;
	m_id = 0;
}

unsigned long long CEventControl::getArriveTime() const
{
	if (m_isTrigger)
	{
		return m_saveTime + m_delay;
	}
	else
	{
		return m_saveTime + m_restrain;
	}
}

EventId CEventControl::getId() const
{
	return m_id;
}

bool CEventControl::isTrigger() const
{
	return m_isTrigger;
}

bool CEventControl::isTimeArrive(unsigned long long runtime) const
{
	if (m_isTrigger && m_saveTime + m_delay < runtime)
	{
		return true;
	}

	return false;
}

bool CEventControl::isRestrain(unsigned long long runtime) const
{
	if (!m_isTrigger && m_saveTime + m_restrain > runtime)
	{
		//未触发、当前时间小于结束时间, 处于抑止状态
		return true;
	}

	return false;
}

} // Event
