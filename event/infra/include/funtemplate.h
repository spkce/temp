
//该文件需要多次引用，此处不能使用宏防止多次引用

#include "def.h"
#include "stdio.h"

#define FUCTION_TEMPLATE MACRO_JOIN(TFuncation, PARAM_NUM)

template <typename R, TEMPLATE_PARAM_TYPE>
class FUCTION_TEMPLATE
{
	class X {};
	enum
	{
		typeEmpty,
		typeMember,
		typePointer,
	};
	typedef R (X::*MemFunc)(PARAM_LIST);
	typedef R (*FuncPtr)(PARAM_LIST);

	union 
	{
		struct 
		{
			MemFunc proc;
			X* obj;
		}memFunc;
		FuncPtr funcPtr;
	}m_func;

	int m_type;
public:
	FUCTION_TEMPLATE():m_type(typeEmpty) {};
	
	template<typename O>
	FUCTION_TEMPLATE(R(O::*f)(PARAM_LIST), const O * o)
	{
		m_func.memFunc.proc = horrible_cast<MemFunc>(f);
		m_func.memFunc.obj = horrible_cast<X*>(o);
		m_type = typeMember;
	}
	
	FUCTION_TEMPLATE(FuncPtr f)
	{
		m_func.funcPtr = f;
		m_type = typePointer;
	}

	template<typename O>
	bool bind(R(O::*f)(PARAM_LIST), const O * o)
	{
		if (m_type == typeEmpty)
		{
			m_func.memFunc.proc = horrible_cast<MemFunc>(f);
			m_func.memFunc.obj = horrible_cast<X*>(o);
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
			m_func.memFunc.proc = NULL;
			m_func.memFunc.obj = NULL;
		}
		else if (m_type == typePointer)
		{
			m_func.funcPtr = NULL;
		}

		m_type = typeEmpty;
	}

	inline FUCTION_TEMPLATE & operator=(const FUCTION_TEMPLATE & fun)
	{
		this->m_type = fun.m_type;
		this->m_func = fun.m_func;
		return *this;
	}

	bool operator==(const FUCTION_TEMPLATE & fun)
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

	inline R operator()(PARAM_LIST)
	{
		if (m_type == typeMember)
		{
			return (m_func.memFunc.obj->*m_func.memFunc.proc)(PARAM_INPUT);
		}
		else if (m_type == typePointer)
		{
			return m_func.funcPtr(PARAM_INPUT);
		}
		return R(0);
	}
	inline bool isEmpty() const
	{
		return (m_type == typeEmpty);
	}
};

#undef FUCTION_TEMPLATE

