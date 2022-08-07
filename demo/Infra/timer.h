#ifndef __TIMER_H__
#define __TIMER_H__

#include "cfunc.h"

namespace Infra
{

struct TimerInternal;

class CTimer
{
public:
	typedef CFunc<void, unsigned long long> TimerProc_t;
public:
	CTimer();
	CTimer(const char* name);
	virtual ~CTimer();
	
	bool setTimerAttr(const TimerProc_t & proc, unsigned int period, unsigned int delay = 0, int times = -1);
	bool setProc(const TimerProc_t & proc);
	bool run();
	bool stop();
	bool isRun();
	
private:

	TimerInternal* m_pInternal;
};


}//Infra

#endif //__TIMER_H__
