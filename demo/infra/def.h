
#ifndef __DEF_H__
#define __DEF_H__

namespace Infra
{
//JOIN
#define MACRO_JOIN(X, Y) MACRO_DO_JOIN(X, Y)
#define MACRO_DO_JOIN(X, Y) MACRO_DO_JOIN2(X, Y)
#define MACRO_DO_JOIN2(X, Y) X##Y

//filename usd to __FILE__
#define filename(x) strrchr(x, '/') ? strrchr(x, '/') + 1 : x

//horrible_cast
template<class OutputClass, class InputClass>
union horrible_union
{
	OutputClass out;
	InputClass in;
};

template<class OutputClass, class InputClass>
inline OutputClass horrible_cast(const InputClass input)
{
	horrible_union<OutputClass, InputClass> u;

	typedef int ERROR_CantUseHorrible_cast[sizeof(InputClass)==sizeof(u)? 1 : -1];
	ERROR_CantUseHorrible_cast dummy;
	(void)dummy;

	u.in = input;
	return u.out;
}


}//Infra

#endif //__DEF_H__
