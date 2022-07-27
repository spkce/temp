#ifndef __EVENT_ID_H__
#define __EVENT_ID_H__

#include "event.h"

namespace Event
{

using EventId = EVENT_ID;

class CEventID
{
public:
	static CEventID* instance();

private:
	CEventID();
	virtual ~CEventID();

public:
	EventId getId();

private:
	unsigned long long m_time;
	unsigned int m_times;
};

} //Event

#endif //__EVENT_ID_H__
