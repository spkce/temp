#ifndef __EVENT_CENTRE_H__
#define __EVENT_CENTRE_H__


#include "EventCtrl.h"

#include "TFuncation.h"

namespace Event
{

struct EventMap
{
	EventType_t type; // �¼�����
	unsigned int delay;	//��ʱʱ�� ms
	unsigned int restrain; //��ֹʱ�� ms
};

class IEvent
{
public:
	typedef Infra::TFuncation4<void, EventType_t, EventId, EventAction_t, unsigned long long> EventWatch_t;

protected:
	IEvent();
	virtual ~IEvent();

public:
	static IEvent* instance();

	virtual bool init(const struct EventMap* p, unsigned int num) = 0;

	virtual bool attach(const EventWatch_t & fun) = 0;
	
	virtual bool detach(const EventWatch_t & fun) = 0;

	virtual bool notify(EventType_t type) = 0;
};

} //Event
#endif //__EVENT_CENTRE_H__
