#include <map>
#include "Log.h"
#include "obsrv.h"
#include "MsgQueue.h"
#include "link.h"
#include "ctime.h"
#include "EventCentre.h"

namespace Event
{

class CEventManager
{
private:
	struct eventPacket
	{
		unsigned long long sysTime;
		EventId id;
		EventType_t type;
		EventAction_t action;
	};
public:
	CEventManager();
	virtual ~CEventManager();

public:
	bool init(const struct EventMap* p, unsigned int num);
	bool attach(const Infra::CObsrv<EventType_t, EventId, EventAction_t, unsigned long long>::watch_t & fun);
	bool detach(const Infra::CObsrv<EventType_t, EventId, EventAction_t, unsigned long long>::watch_t & fun);

	bool triggerEvent(EventType_t type);
private:
	bool sendEvent(EventType_t type, EventId id, EventAction_t action, unsigned long long sysTime);
	void eventProc(void* arg);

private:
	Infra::CObsrv<EventType_t, EventId, EventAction_t, unsigned long long> m_obsrv;
	std::map<EventType_t, CEventControl> m_mapEventCtrl;
	Infra::CMutex m_mutexLink;
	Infra::CLink m_ctrlLink;
	Infra::CMsgQueue m_queue;
	Infra::CThread m_eventThread;
};


CEventManager::CEventManager()
:m_obsrv(20, 16)
,m_mutexLink()
,m_ctrlLink()
,m_queue(20, sizeof(struct eventPacket))
{
	m_eventThread.attachProc(Infra::ThreadProc_t(&CEventManager::eventProc, this));
	m_eventThread.createTread();
}

CEventManager::~CEventManager()
{
	m_eventThread.stop(true);
	m_eventThread.detachProc(Infra::ThreadProc_t(&CEventManager::eventProc, this));
}
 
bool CEventManager::init(const struct EventMap* p, unsigned int num)
{
	for (size_t i = 0; i < num; i++)
	{
		m_mapEventCtrl.insert(std::pair<EventType_t, CEventControl>(p[i].type, CEventControl(p[i].type, p[i].delay,p[i].restrain)));
	}
	
	m_eventThread.run();
	return true;
}

bool CEventManager::attach(const Infra::CObsrv<EventType_t, EventId, EventAction_t, unsigned long long>::watch_t & fun)
{
	return m_obsrv.attach(fun);
}

bool CEventManager::detach(const Infra::CObsrv<EventType_t, EventId, EventAction_t, unsigned long long>::watch_t & fun)
{
	return m_obsrv.detach(fun);
}

bool CEventManager::triggerEvent(EventType_t type)
{
	const unsigned long long runTime = Infra::CTime::getSystemTimeMSecond();
	const unsigned long long sysTime = Infra::CTime::getRealTimeSecond();

	std::map<EventType_t, CEventControl>::iterator iter = m_mapEventCtrl.find(type);
	if (iter != m_mapEventCtrl.end())
	{
		CEventControl* p = &(iter->second);

		if(!p->trigger(runTime))
		{
			return false;
		}
		
		EventId id = p->getId();
		
		if (!sendEvent(type, id, eventAction_begin, sysTime))
		{
			return false;
		}

		m_mutexLink.lock();
		const unsigned int num = m_ctrlLink.linkSize();
		if (num == 0)
		{
			m_ctrlLink.rise((void*)p);
			m_mutexLink.unlock();
			return true;
		}

		for (size_t i = 0; i < num; i++)
		{
			CEventControl* pTemp = (CEventControl*)m_ctrlLink.get(i);
			//按到达时间插入
			if (pTemp->getArriveTime() > p->getArriveTime())
			{
				m_ctrlLink.insert((void*)p, i);
				m_mutexLink.unlock();
				return true;
			}
		}

		//此到达时间最长，加在末尾
		m_ctrlLink.rise((void*)p);
		m_mutexLink.unlock();
		return true;
	}
	else
	{
		EventId id = CEventID::instance()->getId();
		return sendEvent(type, id, eventAction_begin, sysTime);
	}
}

bool CEventManager::sendEvent(EventType_t type, EventId id, EventAction_t action, unsigned long long sysTime)
{
	struct eventPacket packet = {0};

	packet.sysTime = sysTime;
	packet.type = type;
	packet.id = id;
	packet.action = action;

	if(m_queue.input((const char*)&packet, sizeof(struct eventPacket), 1))
	{
		Trace("Event", "event: %d id: %llu send success\n", type, id);
		return true;
	}
	else
	{
		Error("Event", "event: %d id: %llu send fail\n", type, id);
		return false;
	}
}

void CEventManager::eventProc(void* arg)
{
	struct eventPacket packet = {0};
	if (m_queue.output((char *)&packet, sizeof(struct eventPacket), 100) > 0)
	{
		m_obsrv.distribute(packet.type, packet.id, packet.action, packet.sysTime);
		return ;
	}
	else
	{
		const unsigned long long runTime = Infra::CTime::getSystemTimeMSecond();
		const unsigned long long sysTime = Infra::CTime::getRealTimeSecond();
		m_mutexLink.lock();
		CEventControl* p = NULL;
		if (m_ctrlLink.remove((void**)&p, 0) != -1)
		{
			if (p != NULL && p->isTimeArrive(runTime))
			{
				m_mutexLink.unlock();
				m_obsrv.distribute(p->m_type, p->m_id, eventAction_end, sysTime);
				p->clear(runTime);
				return ;
			}
			else if (p != NULL)
			{
				//时间未到插回
				m_ctrlLink.insert((void*)p, 0);
			}
		}
		m_mutexLink.unlock();
	}

	//若无事件处理，延时10ms
	Infra::CTime::delay_ms(10);
}

class CEventCenter : public IEvent
{
public:
	CEventCenter();
	virtual ~CEventCenter();

public:
	virtual bool init(const struct EventMap* p, unsigned int num);
	virtual bool attach(const EventWatch_t & fun);
	virtual bool detach(const EventWatch_t & fun);
	virtual bool notify(EventType_t type);

private:
	void eventProc(void* arg);

private:
	CEventManager m_manager;
	Infra::CThread m_Thread;
	Infra::CMsgQueue m_queue;
};

CEventCenter::CEventCenter()
:m_manager()
,m_queue(20, sizeof(EventType_t))
{
	m_Thread.attachProc(Infra::ThreadProc_t(&CEventCenter::eventProc, this));
	m_Thread.createTread();
}

CEventCenter::~CEventCenter()
{
	m_Thread.stop(true);
	m_Thread.detachProc(Infra::ThreadProc_t(&CEventCenter::eventProc, this));
}

bool CEventCenter::init(const struct EventMap* p, unsigned int num)
{
	if (m_manager.init(p, num))
	{
		m_Thread.run();
		return false;
	}
	
	return true;
}

bool CEventCenter::attach(const EventWatch_t & fun)
{
	return m_manager.attach(fun);
}

bool CEventCenter::detach(const EventWatch_t & fun)
{
	return m_manager.detach(fun);
}

bool CEventCenter::notify(EventType_t type)
{
	if (type >= event_max)
	{
		return false;
	}

	if(m_queue.input((const char*)&type, sizeof(EventType_t), 1))
	{
		return true;
	}
	else
	{
		Error("Event", "event: %d notify fail\n", type);
		return false;
	}
}

void CEventCenter::eventProc(void* arg)
{
	EventType_t type;
	if (m_queue.output((char *)&type, sizeof(EventType_t), 100) > 0)
	{
		m_manager.triggerEvent(type);
		return ;
	}
	//若无事件处理，延时100ms
	Infra::CTime::delay_ms(100);
}

IEvent::IEvent()
{

}

IEvent::~IEvent()
{

}

IEvent* IEvent::instance()
{
	static CEventCenter* pInst = NULL;
	if (pInst == NULL)
	{
		static Infra::CMutex sm_mutex;
		Infra::CGuard<Infra::CMutex> guard(sm_mutex);
		if (pInst == NULL)
		{
			pInst = new CEventCenter;
		}
	}
	return pInst;
}

}//Event
