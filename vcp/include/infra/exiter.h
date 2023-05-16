#ifndef __EXITER_H__
#define __EXITER_H__

namespace Infra
{

class IExiter
{
public:
	typedef void (*ExitFunc)(void*);
	
	static IExiter* intstance();

protected:
	IExiter();
	virtual ~IExiter();

public:
	virtual void attach(ExitFunc exit, void* p) = 0;
};

}//Infra
#endif // __EXITER_H__
