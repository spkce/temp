#ifndef __SESSION_H__
#define __SESSION_H__

#include <vector>
#include <netinet/in.h>
#include "thread.h"
#include "timer.h"

namespace NetFwk
{

/**
* @brief 会话基类
**/
class ISession
{
public:
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
	ISession(){};
	virtual ~ISession(){};

public:
	/**
	* @brief 创建session
	* @return 成功: 会话指针；失败：空指针
	**/
	static ISession* create(int sockfd, struct sockaddr_in* addr, int timeout);

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
	* @brief 获取超时时间
	* @return 超时时间（s）
	**/
	virtual int getTimeout() const = 0;
	
	/**
	* @brief 获取会话状态
	* @return 获取会话状态
	**/
	virtual state_t getState() const = 0;

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
	* @brief 接收消息消息
	* @param buf 消息内容
	* @param len 消息长度
	* @return 发送的数据长度
	**/
	virtual int recv(char* buf, int len) = 0;

	/**
	* @brief 发送消息
	* @param buf 消息内容
	* @param len 消息长度
	* @return 发送的数据长度
	**/
	virtual int send(const char* buf, int len) = 0;
};

} //NetFwk
#endif //__SESSION_H__
