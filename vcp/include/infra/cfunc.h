#ifndef __CFUNC_H__
#define __CFUNC_H__

#include "def.h"
#include <utility>

namespace Infra
{

template <typename R, typename... Args>
class CFunc
{
	class X {};
	enum
	{
		typeEmpty,
		typeMember,
		typePointer,
	};
	typedef R (X::*MemFunc)(Args... args);
	typedef R (*FuncPtr)(Args... args);

	union
	{
		struct
		{
			MemFunc proc;
			X *obj;
		} memFunc;
		FuncPtr funcPtr;
	} m_func;
	int m_type;

public:
	CFunc() : m_type(typeEmpty){};

	template <typename O>
	CFunc(R (O::*f)(Args... args), const O *o)
	{
		m_func.memFunc.proc = horrible_cast<MemFunc>(f);
		m_func.memFunc.obj = horrible_cast<X *>(o);
		m_type = typeMember;
	}

	CFunc(FuncPtr f)
	{
		m_func.funcPtr = f;
		m_type = typePointer;
	}

	template <typename O>
	bool bind(R (O::*f)(Args... args), const O *o)
	{
		if (m_type == typeEmpty)
		{
			m_func.memFunc.proc = horrible_cast<MemFunc>(f);
			m_func.memFunc.obj = horrible_cast<X *>(o);
			m_type = typeMember;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool bind(FuncPtr f)
	{
		if (m_type == typeEmpty)
		{
			m_func.funcPtr = f;
			m_type = typePointer;
			return true;
		}
		else
		{
			return false;
		}
	}

	void unbind()
	{
		if (m_type == typeMember)
		{
			m_func.memFunc.proc = nullptr;
			m_func.memFunc.obj = nullptr;
		}
		else if (m_type == typePointer)
		{
			m_func.funcPtr = nullptr;
		}

		m_type = typeEmpty;
	}

	inline CFunc &operator=(const CFunc &fun)
	{
		this->m_type = fun.m_type;
		this->m_func = fun.m_func;
		return *this;
	}

	bool operator==(const CFunc &fun)
	{
		if (m_type == typeMember && fun.m_type == typeMember)
		{
			return m_func.memFunc.proc == fun.m_func.memFunc.proc && m_func.memFunc.obj == fun.m_func.memFunc.obj;
		}
		else if (m_type == typePointer && fun.m_type == typePointer)
		{
			return m_func.funcPtr == fun.m_func.funcPtr;
		}

		return false;
	}
	template <typename ...Args_>
	inline R operator()(Args_ &&...args)
	{
		if (m_type == typeMember)
		{
			return (m_func.memFunc.obj->*m_func.memFunc.proc)(std::forward<Args_>(args)...);
		}
		else if (m_type == typePointer)
		{
			return m_func.funcPtr(std::forward<Args_>(args)...);
		}
		return R(0);
	}
	inline bool isEmpty() const
	{
		return (m_type == typeEmpty);
	}
};

} // Infra
#endif //__CFUNC_H__
