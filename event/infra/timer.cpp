
#include <unistd.h>
#include <string.h>

//#include "singlenton.h"
#include "link.h"
#include "ctime.h"
#include "timer.h"
#include "thread.h"
#include "stdio.h"
#include "LogInternal.h"

namespace Infra
{

enum
{
	emTimerUnInit = 0,
	emTimerIdle,
	emTimerWait,
	emTimerWork,
};

#define NAME_LEN 32
struct TimerInternal
{
	TimerInternal();
	~TimerInternal();
	void clear();
	inline unsigned long long getTimeout()
	{
		return setupTime + (delay == 0 ? period : delay);
	}
	Infra::CMutex mutex;
	CTimer::TimerProc_t proc;
	unsigned long long setupTime;
	int times;
	unsigned int delay;
	unsigned int period;
	int status;
	bool isIdle;
	char name[NAME_LEN];
	
};

TimerInternal::TimerInternal()
:mutex()
,proc()
,setupTime(0)
,times(0)
,delay(0)
,period(0)
,status(emTimerUnInit)
,isIdle(true)
{
	memset(name, 0, NAME_LEN);
}

TimerInternal::~TimerInternal()
{

}

void TimerInternal::clear()
{
	setupTime = 0;
	times = 0;
	delay = 0;
	period = 0;
	status = emTimerUnInit;
	isIdle = true;
	memset(name, 0, NAME_LEN);
}

class CTimerManger
{
#define PER_TIMER_ALLOCATE 10
public:
	static CTimerManger* instance()
	{
		static CTimerManger* pInstance = NULL;
		if (pInstance == NULL)
		{
			static Infra::CMutex sm_mutex;
			Infra::CGuard<Infra::CMutex> guard(sm_mutex);
			if (pInstance == NULL)
			{
				pInstance = new CTimerManger;
			}
		}
		return pInstance;
	}
private:
	CTimerManger();
	~CTimerManger();
public:
	TimerInternal* allocateTimer();
	void backTimer(TimerInternal* p);
	void setupTimer(TimerInternal* p);
private:
	void allocateIdleTimer(unsigned int n);
	void thread_proc(void* arg);
	static unsigned long long getCurTime();

private:
	CLink m_linkWorkTimer;
	CLink m_linkIdleTimer;
	CMutex m_mutexWorkLink;
	CMutex m_mutexIdleLink;
	unsigned long long m_curTime;
	CThread m_thread;
};

CTimerManger::CTimerManger()
:m_linkWorkTimer()
,m_linkIdleTimer()
,m_mutexWorkLink()
,m_mutexIdleLink()
{
	allocateIdleTimer(PER_TIMER_ALLOCATE);

	m_curTime = getCurTime();
	
	m_thread.attachProc(Infra::ThreadProc_t(&CTimerManger::thread_proc, this));
	m_thread.createTread();
	m_thread.run();
}

CTimerManger::~CTimerManger()
{
	m_thread.stop(true);
	m_thread.detachProc(Infra::ThreadProc_t(&CTimerManger::thread_proc, this));
}

TimerInternal* CTimerManger::allocateTimer()
{
	TimerInternal* p = NULL;

	{
		Infra::CGuard<Infra::CMutex> guard(m_mutexIdleLink);
		if (m_linkIdleTimer.linkSize() == 0)
		{
			allocateIdleTimer(PER_TIMER_ALLOCATE);
		}

		m_linkIdleTimer.reduce((void**)&p);
	}
	p->clear();
	return p;
}

void CTimerManger::backTimer(TimerInternal* p)
{
	Infra::CGuard<Infra::CMutex> guard(m_mutexIdleLink);
	m_linkIdleTimer.rise((void*)p);
}

void CTimerManger::setupTimer(TimerInternal* p)
{
	TimerInternal* pTemp = NULL;
	unsigned int i = 0;
	unsigned int iTemp = (p->delay !=0) ? p->delay : p->period;
	InfraTrace("setup Timer name: %s\n", p->name);
	Infra::CGuard<Infra::CMutex> guard(m_mutexWorkLink);

	const unsigned int uWorkTimerNum = m_linkWorkTimer.linkSize();

	m_curTime = getCurTime();
	InfraTrace("current work timer num = %d\n", uWorkTimerNum);
	InfraTrace("current time = %llu ms\n", m_curTime);

	if (uWorkTimerNum == 0)
	{
		//此定时器是第一个装载
		m_linkWorkTimer.rise((void*)p);
		InfraTrace("insert work link start\n");
		goto timer_set;
	}

	if (uWorkTimerNum > 0)
	{
		for (i = 0; i < uWorkTimerNum; i++)
		{
			pTemp = (TimerInternal*)m_linkWorkTimer.get(i);
			//按timeout时间查找位置
			if (pTemp->getTimeout() > (m_curTime + iTemp))
			{
				m_linkWorkTimer.insert((void*)p, i);
				InfraTrace("insert work link %d\n", i);
				goto timer_set;
			}
		}

		//此定时器timeout时间最长，加在末尾
		m_linkWorkTimer.rise((void*)p);
		InfraTrace("insert work link end\n");
	}

timer_set:
	p->isIdle = false;
	p->setupTime = m_curTime;
	p->status = emTimerWait;
}

void CTimerManger::allocateIdleTimer(unsigned int n)
{
	//创建10个空白定时器，放入空闲队列
	TimerInternal* p = new TimerInternal[n];
	for (unsigned int i = 0; i < n; i++)
	{
		m_linkIdleTimer.rise((void*)(p + i));
	}
}

void CTimerManger::thread_proc(void* arg)
{
	TimerInternal* p = NULL;
	bool isWork;
	unsigned long long timeout = 0;
	//while(loop())
	do
	{
		
		m_mutexWorkLink.lock();
		p = (TimerInternal*)m_linkWorkTimer.get(0);
		if (p == NULL)
		{
			m_mutexWorkLink.unlock();
			usleep(1000);
			continue;
		}
		timeout = p->getTimeout();
		m_mutexWorkLink.unlock();

		m_curTime = getCurTime();

		if (timeout <= m_curTime)
		{
			InfraTrace("curTime = %llu <= Time out：%llu ms \n", m_curTime, timeout);
			m_mutexWorkLink.lock();
			m_linkWorkTimer.remove((void**)&p, 0);
			m_mutexWorkLink.unlock();

			p->mutex.lock();
			isWork = p->times != 0;
			p->mutex.unlock();
			InfraTrace("timer: %s times: %d\n", p->name, p->times);
			if (!p->proc.isEmpty() && isWork)
			{
				p->status = emTimerWork;
				p->proc((int)m_curTime);
				p->status = emTimerWait;

				p->mutex.lock();
				isWork = (p->times < 0 || (--p->times) > 0);
				p->mutex.unlock();

				if (isWork)
				{
					//重新插入工作链表
					setupTimer(p);
				}
				else 
				{
					p->status = emTimerIdle;
				}
			}
		}
		usleep(1000);
	} while(0);
}

unsigned long long CTimerManger::getCurTime()
{
	return CTime::getSystemTimeMSecond();
}

CTimer::CTimer()
{
	m_pInternal = CTimerManger::instance()->allocateTimer();
	m_pInternal->status = emTimerIdle;
	snprintf(m_pInternal->name, NAME_LEN, "timer_%p", this);
}

CTimer::CTimer(const char* name)
{
	m_pInternal = CTimerManger::instance()->allocateTimer();
	m_pInternal->status = emTimerIdle;
	strncpy(m_pInternal->name, name, NAME_LEN - 1);
}

CTimer::~CTimer()
{
	stop();
	CTimerManger::instance()->backTimer(m_pInternal);
}

bool CTimer::setTimerAttr(const TimerProc_t & proc, unsigned int period, unsigned int delay, int times)
{
	if (m_pInternal == NULL || times == 0)
	{
		return false;
	}
	
	m_pInternal->period = period;
	m_pInternal->delay = delay;
	m_pInternal->times = times;
	m_pInternal->proc = proc;
	return true;
}

bool CTimer::setProc(const TimerProc_t & proc)
{
	if (m_pInternal == NULL)
	{
		return false;
	}

	m_pInternal->proc = proc;

	return true;
}

bool CTimer::run()
{
	InfraTrace("CTimer::run \n");
	if (m_pInternal == NULL)
	{
		InfraTrace("m_pInternal == NULL \n");
		return false;
	}
	
	InfraTrace("setup timer : %p \n", m_pInternal);

	CTimerManger::instance()->setupTimer(m_pInternal);
	return true;
}

bool CTimer::stop()
{
	if (m_pInternal->status <= emTimerIdle)
	{
		InfraTrace("timer: %s stop fail\n", m_pInternal->name);
		return false;
	}
	m_pInternal->mutex.lock();
	m_pInternal->times = 0;
	m_pInternal->mutex.unlock();
	InfraTrace("timer: %s stop \n", m_pInternal->name);
	return true;
}

bool CTimer::isRun()
{
	return m_pInternal->status == emTimerWait || m_pInternal->status == emTimerWork;
}

} //Infra
