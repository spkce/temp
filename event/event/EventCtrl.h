#ifndef __EVENT_CTRL_H__
#define __EVENT_CTRL_H__

#include "EventId.h"

namespace Event
{

using EventType_t = EVENT_TYPE_T;
using EventAction_t = EVENT_ACTION_T;

class CEventControl
{
	friend class CEventManager;
public:
	CEventControl(EventType_t type, unsigned int delay, unsigned int restrain);
	CEventControl(const CEventControl & o);
	virtual ~CEventControl();
	bool trigger(unsigned long long runtime);
	void clear(unsigned long long runtime);
	unsigned long long getArriveTime() const;
	EventId getId() const;
	bool isTrigger() const;
	bool isTimeArrive(unsigned long long runtime) const;
	bool isRestrain(unsigned long long runtime) const;

private:
	const EventType_t m_type;
	const unsigned int m_delay;		//延时时间 ms
	const unsigned int m_restrain;	//抑止时间 ms
	unsigned long long m_saveTime; //系统运行时间 ms
	EventId m_id;
	bool m_isTrigger;
	
};


} // Event

#endif //__EVENT_CTRL_H__
