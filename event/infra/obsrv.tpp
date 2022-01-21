
#include <pthread.h>
#include "obsrv.h"
#include "LogInternal.h"
#include <queue>

namespace Infra
{

template <typename P1, typename P2, typename P3, typename P4>
class CWatcher
{
private:
	struct stParam
	{
		stParam(P1 _p1, P2 _p2, P3 _p3, P4 _p4)
		:p1(_p1),p2(_p2),p3(_p3),p4(_p4)
		{}
		stParam(const stParam & o)
		{
			p1 = o.p1;
			p2 = o.p2;
			p3 = o.p3;
			p4 = o.p4;
		}
		P1 p1;
		P2 p2; 
		P3 p3; 
		P4 p4;
	};
public:
	static CWatcher* create(const typename CObsrv<P1, P2, P3, P4>::watch_t & fun, int maxParam)
	{
		return new CWatcher(fun, maxParam);
	}

	void destory()
	{
		while (m_isProcessing)
		{
			
		}
		
		delete this;
	}

private:
	CWatcher(const typename CObsrv<P1, P2, P3, P4>::watch_t & fun, int maxParam)
	:m_maxParam(maxParam), m_mutex(), m_proc(fun),m_isProcessing(false)
	{}

	~CWatcher()
	{

	}

	static void* dispose(void* arg)
	{
		CWatcher* p = (CWatcher*)arg;
		p->m_isProcessing = true;

		p->m_mutex.lock();
		while (p->m_qParam.size() > 0)
		{
			struct stParam stp = p->m_qParam.front();
			p->m_qParam.pop();
			p->m_mutex.unlock();
			if (!p->m_proc.isEmpty())
			{
				p->m_proc(stp.p1, stp.p2, stp.p3, stp.p4);
			}
			p->m_mutex.lock();
		}
		p->m_mutex.unlock();
		p->m_isProcessing = false;
		return NULL;
	}

public:
	bool operator==(const typename CObsrv<P1, P2, P3, P4>::watch_t & fun)
	{
		return m_proc == fun;
	}
	
	bool isRun()
	{
		return m_isProcessing;
	}

	bool run(P1 p1, P2 p2, P3 p3, P4 p4)
	{
		if(m_proc.isEmpty())
		{
			return false;
		}

		m_mutex.lock();
		while (m_qParam.size() >= (size_t)m_maxParam)
		{
			m_qParam.pop();
		}

		m_qParam.push(stParam(p1, p2, p3, p4));
		m_mutex.unlock();

		if(!m_isProcessing)
		{
			int err = pthread_create(&m_handle, NULL, (void*(*)(void*))&CWatcher::dispose, (void*)this);
			if (err)
			{
				//线程创建失败
				InfraTrace("create pthread error: %d\n",err);
				return false;
			}

			//设置线程为可分离状态，线程运行结束后会自动释放资源。
			if (pthread_detach(m_handle))
			{
				InfraTrace("detach pthread error: %d\n",err);
				return false;
			}
		}
		return true;
	}

private:
	const int m_maxParam;
	Infra::CMutex m_mutex;
	std::queue<struct stParam> m_qParam;
	typename CObsrv<P1, P2, P3, P4>::watch_t m_proc;
	pthread_t m_handle;
	bool m_isProcessing;

};

template <typename P1, typename P2, typename P3, typename P4>
CObsrv<P1, P2, P3, P4>::CObsrv(int maxWather, int maxParam)
:m_maxWather(maxWather)
,m_maxParam(maxParam)
{
}

template <typename P1, typename P2, typename P3, typename P4>
CObsrv<P1, P2, P3, P4>::~CObsrv()
{
}

template <typename P1, typename P2, typename P3, typename P4>
bool CObsrv<P1, P2, P3, P4>::attach(const watch_t & fun)
{
	CGuard<CMutex> guard(m_mutex);
	if (m_vWatcher.size() >= (size_t)m_maxWather)
	{
		return false;
	}

	for (typename std::vector<CWatcher<P1, P2, P3, P4>*>::iterator it = m_vWatcher.begin(); it != m_vWatcher.end(); it++)
	{
		if (*(*it) == fun)
		{
			return false;
		}
	}

	CWatcher<P1, P2, P3, P4>* p = CWatcher<P1, P2, P3, P4>::create(fun, m_maxParam);
	m_vWatcher.push_back(p);
	return true;
}

template <typename P1, typename P2, typename P3, typename P4>
bool CObsrv<P1, P2, P3, P4>::detach(const watch_t & fun)
{
	CGuard<CMutex> guard(m_mutex);
	for (typename std::vector<CWatcher<P1, P2, P3, P4>*>::iterator it = m_vWatcher.begin(); it != m_vWatcher.end(); it++)
	{
		CWatcher<P1, P2, P3, P4> & watcher = (*(*it));
		if (watcher == fun && !watcher.isRun())
		{
			watcher.destory();
			m_vWatcher.erase(it);
			return true;
		}
	}
	return false;
}

template <typename P1, typename P2, typename P3, typename P4>
void CObsrv<P1, P2, P3, P4>::distribute(P1 p1, P2 p2, P3 p3, P4 p4)
{
	CGuard<CMutex> guard(m_mutex);
	for (typename std::vector<CWatcher<P1, P2, P3, P4>*>::iterator it = m_vWatcher.begin(); it != m_vWatcher.end(); it++)
	{
		CWatcher<P1, P2, P3, P4> & watcher = (*(*it));
		watcher.run(p1, p2, p3, p4);
	}
}

}//Infra
