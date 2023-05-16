#include <stdlib.h>
#include <list>
#include <memory>
#include "LogInternal.h"
#include "thread.h"
#include "exiter.h"

namespace Infra
{

class CExiter : public IExiter
{
	struct Exiter
	{
		Exiter(ExitFunc exit, void* p)
		:pfunc(exit)
		,pData(p)
		{
		}
		Exiter(const Exiter & exiter)
		{
			pfunc = exiter.pfunc;
			pData = exiter.pData;
		}
		ExitFunc pfunc;
		void* pData;
	};
public:
	CExiter();
	virtual ~CExiter();

	virtual void attach(ExitFunc exit, void* p);
	

private:
	static void exit();

	Infra::CMutex m_mutex;
	std::list<struct Exiter> m_listExiter;

};

static CExiter* g_pIntstance = nullptr;

CExiter::CExiter()
{
	if(atexit(CExiter::exit) != 0)
	{
		InfraTrace("atexit faild\n");
	}
}

CExiter::~CExiter()
{

}

void CExiter::attach(ExitFunc exit, void* p)
{
	Infra::CGuard<Infra::CMutex> guard(m_mutex);
	m_listExiter.push_front(Exiter(exit, p));
}

void CExiter::exit()
{
	CExiter* p = g_pIntstance;
	if (p == nullptr)
	{
		return ;
	}

	for (auto stExit : p->m_listExiter)
	{
		stExit.pfunc(stExit.pData);
	}
	
	delete g_pIntstance;
}


IExiter* IExiter::intstance()
{
	if (g_pIntstance == nullptr)
	{
		static Infra::CMutex sm_mutex;
		Infra::CGuard<Infra::CMutex> guard(sm_mutex);
		if (g_pIntstance == nullptr)
		{
			g_pIntstance = new CExiter;
		}
	}
	return g_pIntstance;
}

IExiter::IExiter()
{

}

IExiter::~IExiter()
{

}

}// Infra
