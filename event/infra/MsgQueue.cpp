
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include "MsgQueue.h"
#include "ctime.h"
#include "LogInternal.h"
#include <string>
#include <sys/types.h>
#include <unistd.h>

namespace Infra
{
struct QueueInternal
{
	QueueInternal(): qId(-1), name("/") {}
	mqd_t qId;
	std::string name;
};

CMsgQueue::CMsgQueue(const char* name, int maxMsg, int maxMsgLen)
{
	m_pInternal = new QueueInternal();

	struct mq_attr attr = {0};
	attr.mq_msgsize = maxMsgLen;
	attr.mq_maxmsg = maxMsg;

	m_pInternal->name += name;
	InfraTrace("create queue: %s \n", m_pInternal->name.c_str());

	mq_unlink(m_pInternal->name.c_str());
	m_pInternal->qId = mq_open(m_pInternal->name.c_str(), O_CREAT | O_RDWR | O_EXCL, 0664, &attr);
	if (m_pInternal->qId < 0)
	{
		InfraTrace("create queue: %s fail\n", m_pInternal->name.c_str());
	}
}

CMsgQueue::CMsgQueue(int maxMsg, int maxMsgLen)
{
	char name[32] = {0};
	m_pInternal = new QueueInternal();
	// PID + 堆区地址 可以保证该名字在系统中唯一
	snprintf(name, size_t(name), "%d_%p", getpid(), m_pInternal);

	struct mq_attr attr = {0};
	attr.mq_msgsize = maxMsgLen;
	attr.mq_maxmsg = maxMsg;
	m_pInternal->name += name;

	InfraTrace("create queue: %s \n", m_pInternal->name.c_str());

	mq_unlink(m_pInternal->name.c_str());
	m_pInternal->qId = mq_open(m_pInternal->name.c_str(), O_CREAT | O_RDWR | O_EXCL, 0664, &attr);
	if (m_pInternal->qId < 0)
	{
		InfraTrace("create queue: %s fail\n", m_pInternal->name.c_str());
	}
}

CMsgQueue::~CMsgQueue()
{
	mq_close(m_pInternal->qId);
	mq_unlink(m_pInternal->name.c_str());
	delete m_pInternal;
}

bool CMsgQueue::input(const char *msg, size_t len, int timeout, unsigned int prio)
{
	if (msg == NULL || m_pInternal->qId < 0)
	{
		InfraTrace("Queue %s not ready\n", m_pInternal->name.c_str());
		return false;
	}

	if (timeout < 0)
	{
		return mq_send(m_pInternal->qId, msg, len, prio) == 0;
	}
	else
	{
		struct timespec absTime = {0};
		CTime::covertRealTime((unsigned int)timeout, &absTime);
		return mq_timedsend(m_pInternal->qId, msg, len, prio, &absTime) == 0;
	}
}

int CMsgQueue::output(char *msg, size_t len, int timeout, unsigned int *priop)
{
	if (msg == NULL || m_pInternal->qId < 0)
	{
		InfraTrace("Queue %s not ready\n", m_pInternal->name.c_str());
		return false;
	}

	if (timeout < 0)
	{
		return mq_receive(m_pInternal->qId, msg, len, priop);
	}
	else if (timeout == 0)
	{
		struct timespec absTime = {0};
		return mq_timedreceive(m_pInternal->qId, msg, len, priop, &absTime);
	}
	else
	{
		struct timespec absTime = {0};
		CTime::covertRealTime((unsigned int)timeout, &absTime);
		return mq_timedreceive(m_pInternal->qId, msg, len, priop, &absTime);
	}
}

} //Infra
