
#ifndef __OBSRV_h__
#define __OBSRV_h__

#include "thread.h"
#include <vector>

namespace Infra
{

template <typename P1, typename P2, typename P3, typename P4>
class CWatcher;

template <typename P1, typename P2, typename P3, typename P4>
class CObsrv
{
public:
	typedef TFuncation4<void, P1, P2, P3, P4> watch_t;
public:
	CObsrv(int maxWather, int maxParam);
	virtual ~CObsrv();

	bool attach(const watch_t & fun);
	bool detach(const watch_t & fun);
	void distribute(P1 p1, P2 p2, P3 p3, P4 p4);

private:
	const int m_maxWather;
	const int m_maxParam;
	CMutex m_mutex;
	std::vector<CWatcher<P1, P2, P3, P4>*> m_vWatcher;
};

}//Infra

#include "obsrv.tpp"

#endif //__OBSRV_h__
