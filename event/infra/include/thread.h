#ifndef __THREAD_H__
#define __THREAD_H__
#include "TFuncation.h"

namespace Infra
{

struct MutexInternal;
struct CondInternal;
struct RwlockInternal;
struct ThreadInternal;

/**
* @brief ������
**/
class CMutex
{
public:
	CMutex();
	virtual ~CMutex();
	/**
	* @brief ����������
	* @return true:�ɹ���false:ʧ��
	**/
	bool lock();
	/**
	* @brief ���Լ�����������
	* @return true:�ɹ���false:ʧ��
	**/
	bool trylock();
	/**
	* @brief ����
	* @return true:�ɹ���false:ʧ��
	**/
	bool unlock();
private:
	struct MutexInternal* m_pInternal;
};

/**
* @brief ������������
**/
template <class T>
class CGuard
{
public:
	inline CGuard(T & lock):m_lock(lock)
	{
		m_lock.lock();
	}
	inline ~CGuard()
	{
		m_lock.unlock();
	}
private:
	T & m_lock;
};

/**
* @brief ��������
**/
class CCondSignal
{
public:
	CCondSignal();
	virtual ~CCondSignal();
	/**
	* @brief ����
	* @return true:�ɹ���false:ʧ��
	**/
	bool wait();
	/**
	* @brief �������
	* @return true:�ɹ���false:ʧ��
	**/
	bool signal();
private:
	struct CondInternal* m_pInternal;
};

/**
* @brief ��д��
**/
class CRwlock
{
public:
	CRwlock();
	virtual ~CRwlock();
	/**
	* @brief �Ӷ���������
	* @return true:�ɹ���false:ʧ��
	**/
	bool rLock();
	/**
	* @brief ��д��������
	* @return true:�ɹ���false:ʧ��
	**/
	bool wLock();
	/**
	* @brief ���ԼӶ�����������
	* @return true:�ɹ���false:ʧ��
	**/
	bool tryRLock();
	/**
	* @brief ���Լ�д����������
	* @return true:�ɹ���false:ʧ��
	**/
	bool tryWLock();
	/**
	* @brief ����
	* @return true:�ɹ���false:ʧ��
	**/
	bool unLock();
private:
	struct RwlockInternal* m_pInternal;
};

typedef TFuncation1<void, void *> ThreadProc_t;
/**
* @brief �̻߳���
**/
class IThread
{
friend struct ThreadInternal;
protected:
	IThread();
	virtual ~IThread();
	int create(struct ThreadInternal* pInternal, bool isDetach);
	struct ThreadInternal* allocateThread();
	void releaseThread(struct ThreadInternal* pInternal);
	
	ThreadProc_t m_proc;
	struct ThreadInternal* m_pInternal;
};

/**
* @brief �߳���
**/
class CThread : public IThread
{
public:
	CThread();
	virtual ~CThread();
	/**
	* @brief �����̣߳��������̴߳��ڹ���״̬
	* @param isBlock �Ƿ��������ķ�ʽ�����̡߳�
	* @return true:�ɹ���false:ʧ��
	**/
	bool createTread(bool isBlock = false);

	/**
	* @brief �߳̿�ʼ����
	* @param isLoop �Ƿ�ѭ�����С�
	**/
	void run(bool isLoop = true);

	/**
	* @brief �̹߳���
	* @param isBlock �Ƿ�ȴ��̹߳����ŷ���
	**/
	void suspend(bool isBlock = false);

	/**
	* @brief �����̼߳�������
	**/
	void pasue();

	/**
	* @brief �߳���ֹ����
	* @param isBlock �Ƿ�ȴ��߳��˳���ŷ���
	* @return true:�ɹ���false:ʧ��
	**/
	bool stop(bool isBlock = false);

	/**
	* @brief ע���߳�ִ�к���
	* @param proc ִ�к���
	* @return true:�ɹ���false:ʧ��
	**/
	bool attachProc(const ThreadProc_t & proc);

	/**
	* @brief ע���߳�ִ�к���
	* @param proc �Ѿ�ע�����ִ�к���
	* @return true:�ɹ���false:ʧ��
	**/
	bool detachProc(const ThreadProc_t & proc);

	/**
	* @brief �߳��Ƿ��Ѿ��ɹ�������createTread��
	* @return true:�Ѵ�����false:δ����
	**/
	bool isTreadCreated() const;
};

} //Infra

#endif //__THREAD_H__
