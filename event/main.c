#include "stdio.h"
#include <unistd.h>
#include "event.h"

void func1(EVENT_TYPE_T event, EVENT_ID id, EVENT_ACTION_T action, unsigned long long eventTime)
{
	printf("\033[35m""func1 event:%d id:%lld action:%d time:%lld""\033[0m\n", event, id, action, eventTime);
}

void func2(EVENT_TYPE_T event, EVENT_ID id, EVENT_ACTION_T action, unsigned long long eventTime)
{
	printf("\033[35m""func2 event:%d id:%lld action:%d time:%lld""\033[0m\n", event, id, action, eventTime);
}

void func3(EVENT_TYPE_T event, EVENT_ID id, EVENT_ACTION_T action, unsigned long long eventTime)
{
	printf("\033[35m""func3 event:%d id:%lld action:%d time:%lld""\033[0m\n", event, id, action, eventTime);
}

int main(int argc, char const *argv[])
{
	//event_init();

	event_attach(func1);
	event_attach(func2);
	event_attach(func3);

	int i = 0;

	event_notify(event_BSD);
	while (i < 12)
	{
		if (i++ == 10)
		{
			//event_notify(event_BSD);
			event_detach(func1);
		}
		
		usleep(100000);
	}
	sleep(4);
	event_notify(event_BSD);

	return 0;
}

int syslog_send_data(const char* a, unsigned int b)
{
	return 0;
}