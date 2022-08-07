#ifndef __SESSION_H__
#define __SESSION_H__

#include "thread.h"
#include "timer.h"
#include <vector>
//#include <sys/socket.h>

struct sockaddr_in;

namespace NetFwk
{

/**
* @brief 会话基类
**/
class ISession
{
public:
	/**
	* @brief session 回调函数类型
	**/
	typedef Infra::CFunc<void, ISession*, char*, int> SessionProc_t;
	
	/**
	* @brief session 状态
	**/
	typedef  enum state_t
	{
		emStateCreated,
		emStateLogout,
		emStateLogin,
		emStateClose,
		emStateNone,
	}state_t;
protected:
	/**
	* @brief 构造函数， 设定protected权限，不可实例，可继承
	**/
	ISession(){};
	
	/**
	* @brief 析构函数， 设定protected权限，不可实例，可继承
	**/
	virtual ~ISession(){};

public:
	/**
	* @brief 绑定回调函数
	* @param proc 回调函数
	* @return 成功：true；失败：false
	**/
	virtual bool bind(const SessionProc_t & proc) = 0;

	/**
	* @brief 解绑绑回调函数
	* @return 成功：true；失败：false
	**/
	virtual bool unbind() = 0;

	/**
	* @brief 登录
	* @return 成功：true；失败：false
	**/
	virtual bool login() = 0;

	/**
	* @brief 登出
	* @return 成功：true；失败：false
	**/
	virtual bool logout() = 0;
	
	/**
	* @brief 保活
	* @return 成功：true；失败：false
	**/
	virtual bool keepAlive() = 0;
	
	/**
	* @brief 是否超时
	* @return 超时：true；未超时：false
	**/
	virtual bool isTimeout() = 0;

	/**
	* @brief 获取会话状态
	* @return 获取会话状态
	**/
	virtual state_t getState() = 0;

	/**
	* @brief 关闭会话
	* @return 成功：true；失败：false
	**/
	virtual bool close() = 0;

	/**
	* @brief 会话释放
	**/
	virtual void destroy() = 0;

	/**
	* @brief 发送消息，同步
	* @param buf 消息内容
	* @param len 消息长度
	* @return 发送的数据长度
	**/
	virtual int send(const char* buf, int len) = 0;

};

/**
* @brief 会话管理类
**/
class CSessionManager
{
private:
	/**
	* @brief 构造函数， 设定private权限，外部不可实例，不可继承
	**/
	CSessionManager();
	/**
	* @brief 析构函数， 设定private权限，外部不可实例，不可继承
	**/
	~CSessionManager();

public:
	/**
	* @brief 获取会话管理类实例
	**/
	static CSessionManager* instance();

	/**
	* @brief 创建session
	* @param sockfd 套接字句柄
	* @param addr 对端地址
	* @param timeout 超时时间单位秒，默认值-1 不会超时
	* @return 会话基类指针
	**/
	ISession* createSession(int sockfd, struct sockaddr_in* addr, int timeout = -1);
	/**
	* @brief 取消session
	* @param session 会话基类指针
	**/
	bool cancelSession(ISession* session);
	/**
	* @brief 查询session是否注册中
	* @param session 指针
	* @return 是否注册
	**/
	bool isSessionRegister(ISession* session);
	/**
	* @brief 查询session状态
	* @param session 指针
	* @return 状态
	**/
	int getState(ISession* session);
private:
	/**
	* @brief 注册session
	* @param session 会话基类指针
	**/
	void registerSession(ISession* session);
	/**
	* @brief 定时器回调函数
	* @param arg 时间，系统运行时间毫秒
	**/
	void timerProc(unsigned long long arg);

	Infra::CTimer m_timer;
	Infra::CMutex m_mutex;
	std::vector<ISession*> m_vecSession;
	
};



} //NetFwk
#endif //__SESSION_H__