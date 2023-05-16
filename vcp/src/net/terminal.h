#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include <string>
#include <list>
#include "Session.h"
#include "thread.h"
#include "Protocol.h"

namespace NetFwk
{
class INetServer;

class ITerminal
{

protected:
	/**
	* @param max 最大连接数，
	* @param recvLen 接收缓冲区大小
	**/
	ITerminal(unsigned int max, unsigned int recvLen);
	virtual ~ITerminal();
	
public:
	/**
	* @brief 初始化
	* @param port 服务器端口
	* @param timeout 链接超时时间，-1不超时
	* @return 成功：true；失败：false
	**/
	virtual bool init(unsigned int port, int timeout);

	/**
	* @brief 服务器停止
	* @return 成功：true；失败：false
	**/
	virtual bool stop();

	/**
	* @brief 协议创建函数，由子类实现
	* @return 协议对象
	**/
	virtual IProtocol* createProtocol(ISession* session, size_t recvLen) = 0;

private:
	bool serverTask(int sockfd, struct sockaddr_in* addr);
	void managerTask(void* arg);

protected:
	Infra::CThread m_thread;
	Infra::CMutex m_mutex;
	std::list<IProtocol*> m_listProtocol;
	INetServer* m_pServer;
	const unsigned int m_maxSession;
	const unsigned int m_recvLen;
	unsigned int m_port;

private:
	int m_timeout;
};

} // NetFwk
#endif //__TERMINAL_H__
