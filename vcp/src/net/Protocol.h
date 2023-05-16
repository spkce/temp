#ifndef __PROTOCL_H__
#define __PROTOCL_H__

#include <string>
#include "thread.h"
#include "timer.h"
#include "Session.h"

namespace NetFwk
{

class IProtocol
{
	friend class ITerminal;
public:
	enum
	{
		OK,
		Logout,
	};

protected:
	/**
	* @param session 会话
	* @param recvlen 接收缓冲区大小
	**/
	IProtocol(ISession* session, size_t recvlen);
	virtual ~IProtocol();

public:
	/**
	* @brief 开始
	* @return 成功：true；失败：false
	**/
	virtual bool start();

	/**
	* @brief 回报解析函数，由子类实现
	* @return 
	**/
	virtual int parse(const unsigned char* buf, int len) = 0;
	
	/**
	* @brief 发送报文
	* @param buf 发送内容
	* @param len 发送内容长度
	* @return 剩余未发送的长度
	**/
	virtual int send(const char* buf, int len);

protected:
	/**
	* @brief 登录
	* @return 成功：true；失败：false
	**/
	virtual bool login();

	/**
	* @brief 登出
	* @return 成功：true；失败：false
	**/
	virtual bool logout();
	
	/**
	* @brief 保活
	* @return 成功：true；失败：false
	**/
	virtual bool keepAlive();

private:
	bool isLogout() const;
	void sessionTask(void* arg);

protected:
	Infra::CThread m_thread;
	Infra::CTimer m_timer;
	ISession* m_session;
	const size_t m_recvLen;
	unsigned char* m_pBuffer;
};


} //NetFwk

#endif //__I_PROTOCL_H__
