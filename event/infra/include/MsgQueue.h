#ifndef __MSG_QUEUE_H__
#define __MSG_QUEUE_H__

#include "stdio.h"

namespace Infra
{

struct QueueInternal;

class CMsgQueue
{
public:
	CMsgQueue(const char* name, int maxMsg, int maxMsgLen);
	CMsgQueue(int maxMsg, int maxMsgLen);
	virtual ~CMsgQueue();

public:
	bool input(const char *msg, size_t len, int timeout = -1, unsigned int prio = 0);
	int output(char *msg, size_t len, int timeout = -1, unsigned int *priop = NULL);

private:
	struct QueueInternal * m_pInternal;
};

} //Infra

#endif //__MSG_QUEUE_H__
