
#include "event.h"
#include "EventCentre.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

const struct Event::EventMap eventMap[] =
{
	{event_BSD, 3000, 1000},
};

int event_init()
{
	return Event::IEvent::instance()->init(eventMap, sizeof(eventMap)/sizeof(eventMap[0]));
}

int event_attach(EVENT_OBSERVER fun)
{
	return Event::IEvent::instance()->attach(fun);
}

int event_detach(EVENT_OBSERVER fun)
{
	return Event::IEvent::instance()->detach(fun);
}

int event_notify(EVENT_TYPE_T event)
{
	return Event::IEvent::instance()->notify(event);
}

#ifdef __cplusplus
}
#endif //__cplusplus
