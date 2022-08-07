#ifndef __NET_SERVER_H__
#define __NET_SERVER_H__

#include "thread.h"
#include "Session.h"

//struct sockaddr_in;

namespace NetFwk
{

/**
 * @brief 服务器接口类
*/
class INetServer
{
public:
	/**
	* @brief 服务器回调函数定义 void fun(int fd, struct sockaddr_in* addr);
	* @param fd 套接字句柄
	* @param addr 对端地址
	**/
	typedef Infra::CFunc<bool, int, struct sockaddr_in*> ServerProc_t;
	/**
	* @brief 服务器类型枚举
	**/
	typedef enum Type_t
	{
		emTCPServer,
		emUDPServer,
	}Type_t;
protected:
	/**
	* @brief INetServer构造函数，权限设定protected，禁止实例
	**/
	INetServer();
	/**
	* @brief INetServer析造函数，权限设定protected，禁止实例
	**/
	virtual ~INetServer();
public:
	/**
	* @brief 服务器创建函数
	* @param port 服务器绑定的端口
	* @param type 服务器类型
	* @return 服务器接口
	**/
	static INetServer* create(unsigned int port, Type_t type);
	/**
	* @brief 服务器开始运行
	* @param maxlisten 最大连接
	* @return 成功/失败
	**/
	virtual bool start(unsigned int maxlisten) = 0;
	/**
	* @brief 注册服务器回调函数
	* @param proc 服务器回调函数
	* @return 成功/失败
	**/
	virtual bool attach(ServerProc_t proc) = 0;
	/**
	* @brief 服务器停止运行
	* @return 成功/失败
	**/
	virtual bool stop() = 0;

	/**
	* @brief 服务器是否运行
	* @return 是/否
	**/
	virtual bool isRun() = 0;
};





} //NetFwk
#endif //__NET_SERVER_H__
