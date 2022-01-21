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
* @brief 互斥锁
**/
class CMutex
{
public:
	CMutex();
	virtual ~CMutex();
	/**
	* @brief 加锁，阻塞
	* @return true:成功；false:失败
	**/
	bool lock();
	/**
	* @brief 尝试加锁，不阻塞
	* @return true:成功；false:失败
	**/
	bool trylock();
	/**
	* @brief 解锁
	* @return true:成功；false:失败
	**/
	bool unlock();
private:
	struct MutexInternal* m_pInternal;
};

/**
* @brief 互斥锁辅助器
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
* @brief 条件变量
**/
class CCondSignal
{
public:
	CCondSignal();
	virtual ~CCondSignal();
	/**
	* @brief 阻塞
	* @return true:成功；false:失败
	**/
	bool wait();
	/**
	* @brief 解除阻塞
	* @return true:成功；false:失败
	**/
	bool signal();
private:
	struct CondInternal* m_pInternal;
};

/**
* @brief 读写锁
**/
class CRwlock
{
public:
	CRwlock();
	virtual ~CRwlock();
	/**
	* @brief 加读锁，阻塞
	* @return true:成功；false:失败
	**/
	bool rLock();
	/**
	* @brief 加写锁，阻塞
	* @return true:成功；false:失败
	**/
	bool wLock();
	/**
	* @brief 尝试加读锁，不阻塞
	* @return true:成功；false:失败
	**/
	bool tryRLock();
	/**
	* @brief 尝试加写锁，不阻塞
	* @return true:成功；false:失败
	**/
	bool tryWLock();
	/**
	* @brief 解锁
	* @return true:成功；false:失败
	**/
	bool unLock();
private:
	struct RwlockInternal* m_pInternal;
};

typedef TFuncation1<void, void *> ThreadProc_t;
/**
* @brief 线程基类
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
* @brief 线程类
**/
class CThread : public IThread
{
public:
	CThread();
	virtual ~CThread();
	/**
	* @brief 创建线程，创建后线程处于挂起状态
	* @param isBlock 是否以阻塞的方式创建线程。
	* @return true:成功；false:失败
	**/
	bool createTread(bool isBlock = false);

	/**
	* @brief 线程开始运行
	* @param isLoop 是否循环运行。
	**/
	void run(bool isLoop = true);

	/**
	* @brief 线程挂起
	* @param isBlock 是否等待线程挂起后才返回
	**/
	void suspend(bool isBlock = false);

	/**
	* @brief 挂起线程继续运行
	**/
	void pasue();

	/**
	* @brief 线程终止运行
	* @param isBlock 是否等待线程退出后才返回
	* @return true:成功；false:失败
	**/
	bool stop(bool isBlock = false);

	/**
	* @brief 注册线程执行函数
	* @param proc 执行函数
	* @return true:成功；false:失败
	**/
	bool attachProc(const ThreadProc_t & proc);

	/**
	* @brief 注册线程执行函数
	* @param proc 已经注册过的执行函数
	* @return true:成功；false:失败
	**/
	bool detachProc(const ThreadProc_t & proc);

	/**
	* @brief 线程是否已经成功创建（createTread）
	* @return true:已创建；false:未创建
	**/
	bool isTreadCreated() const;
};

} //Infra

#endif //__THREAD_H__
