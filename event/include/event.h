#ifndef __EVENT_H__
#define __EVENT_H__

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

typedef enum
{
	event_BSD,
	event_max
}EVENT_TYPE_T;

typedef enum
{
	eventAction_begin,
	eventAction_end
}EVENT_ACTION_T;


typedef unsigned long long EVENT_ID;

typedef void (*EVENT_OBSERVER)(EVENT_TYPE_T event, EVENT_ID id, EVENT_ACTION_T action, unsigned long long eventTime);

int event_init();
int event_attach(EVENT_OBSERVER fun);
int event_detach(EVENT_OBSERVER fun);
int event_notify(EVENT_TYPE_T event);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__EVENT_H__
