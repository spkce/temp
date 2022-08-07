#include <stdio.h>
#include <unistd.h>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "string.h"
#include "NetServer.h"
#include "Log.h"


namespace NetFwk
{

class CTcpServer : public INetServer
{
	friend class INetServer;

private:
	/**
	* @brief CTcpServer构造函数，权限设定private，禁止外部实例，禁止继承
	**/
	CTcpServer(unsigned int port);
	/**
	* @brief CTcpServer析造函数，权限设定private，禁止外部实例，禁止继承
	**/
	virtual ~CTcpServer();
	/**
	* @brief 获取TCP服务器实例，若未创建则创建实例
	* @param port 服务器端口号
	* @return TCP服务器实例
	**/
	static CTcpServer* getServer(unsigned int port);
	/**
	* @brief 关闭CP服务器
	* @param port 服务器端口号
	* @return 成功/失败
	**/
	static bool closeServer(unsigned int port);
public:
	/**
	* @brief 注册服务器回调函数
	* @param proc 服务器回调函数
	* @return 成功/失败
	**/
	virtual bool attach(INetServer::ServerProc_t proc);
	/**
	* @brief 服务器开始运行
	* @param maxlisten 最大连接
	* @return 成功/失败
	**/
	virtual bool start(unsigned int maxlisten);
	/**
	* @brief 服务器停止运行
	* @return 成功/失败
	**/
	virtual bool stop();
	/**
	* @brief 服务器是否运行
	* @return 是/否
	**/
	virtual bool isRun();

private:
	/**
	* @brief 线程回调函数,
	* @param arg 线程回调函数显示固定参数，不使用
	* @return 成功/失败
	**/
	void server_task(void* arg);

	INetServer::ServerProc_t m_proc; //服务器回调函数,由attach注册
	Infra::CThread* m_pThread; //服务器线程
	int m_sockfd; //套接字句柄
	int m_port;	//服务器端口号
	
	//Infra::CMutex m_mutex;

	static Infra::CMutex sm_mutex; //静态成员 互斥锁，用于保护sm_mapServer
	static std::map<unsigned int, CTcpServer*> sm_mapServer; //静态成员sm_mapServer 管理所有CTcpServer服务器实例
};

Infra::CMutex CTcpServer::sm_mutex;
std::map<unsigned int, CTcpServer*> CTcpServer::sm_mapServer;

/**
* @brief CTcpServer构造函数，权限设定private，禁止外部实例，禁止继承
**/
CTcpServer::CTcpServer(unsigned int port)
:m_sockfd(-1)
,m_port(port)
{
	m_pThread = new Infra::CThread();
	m_pThread->attachProc(Infra::ThreadProc_t(&CTcpServer::server_task, this));
	m_pThread->createTread();
	
}

/**
* @brief CTcpServer析造函数，权限设定private，禁止外部实例，禁止继承
**/
CTcpServer::~CTcpServer()
{
	stop();
	m_pThread->detachProc(Infra::ThreadProc_t(&CTcpServer::server_task, this));

	delete m_pThread;
	m_pThread = NULL;
}

/**
* @brief 获取TCP服务器实例，若未创建则创建实例
* @param port 服务器端口号
* @return TCP服务器实例
**/
CTcpServer* CTcpServer::getServer(unsigned int port)
{
	Infra::CGuard<Infra::CMutex> guard(sm_mutex);
	std::map<unsigned int, CTcpServer*>::iterator iter = sm_mapServer.find(port);
	if (iter == sm_mapServer.end())
	{
		CTcpServer* p = new CTcpServer(port);
		sm_mapServer.insert(std::pair<unsigned int, CTcpServer*>(port, p));
		return p;
	}

	return iter->second;
}

/**
* @brief 关闭CP服务器
* @param port 服务器端口号
* @return 成功/失败
**/
bool CTcpServer::closeServer(unsigned int port)
{
	Infra::CGuard<Infra::CMutex> guard(sm_mutex);
	std::map<unsigned int, CTcpServer*>::iterator iter = sm_mapServer.find(port);
	if (iter != sm_mapServer.end())
	{
		sm_mapServer.erase(iter);
		return true;
	}

	return false;
}

/**
* @brief 注册服务器回调函数
* @param proc 服务器回调函数
* @return 成功/失败
**/
bool CTcpServer::attach(INetServer::ServerProc_t proc)
{
	if (!m_proc.isEmpty())
	{
		return false;
	}
	m_proc = proc;
	return true;
}

/**
* @brief 服务器开始运行
* @param maxlisten 最大连接
* @return 成功/失败
**/
bool CTcpServer::start(unsigned int maxlisten)
{
	if (m_sockfd >= 0)
	{
		Infra::Error("NetTerminal", "socket is Already open\n");
		return false;
	}

	m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sockfd < 0)
	{
		Infra::Error("NetTerminal", "open socket fail\n");
		return false;
	}
	struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(m_port);
	if (bind(m_sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
	{
		Infra::Error("NetTerminal", "socket bind port:%d fail\n", m_port);
		return false;
	}

	struct timeval timeout={3,0};    //设置超时时间为3秒
	setsockopt(m_sockfd, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

	if(listen(m_sockfd, maxlisten) < 0)
	{
		Infra::Error("NetTerminal", "socket listen port: %d fail\n", m_port);
		return false;
	}
	m_pThread->run();
	Infra::Debug("NetTerminal", "tcp server: %d is ready\n", m_port);
	return true;
}

/**
* @brief 服务器停止运行
* @return 成功/失败
**/
bool CTcpServer::stop()
{
	if (m_sockfd >= 0)
	{
		closeServer(m_port);
		::close(m_sockfd);
		m_sockfd = -1;
		return m_pThread->stop(true);
	}

	return false;
}


/**
* @brief 服务器是否运行
* @return 是/否
**/
bool CTcpServer::isRun()
{
	return m_sockfd >= 0;
}

/**
* @brief 线程回调函数,
* @param arg 线程回调函数固定参数，不使用
* @return 成功/失败
**/
void CTcpServer::server_task(void* arg)
{

	struct sockaddr_in cliaddr = {0};
	socklen_t clilen = sizeof(struct sockaddr_in);

	//if(uWaitMsec != WAIT_FOREVER)
	//{
	//	stTimeout.tv_sec = uWaitMsec/1000;
	//stTimeout.tv_usec = uWaitMsec%1000;
    //    FD_ZERO(&rset);
    //    FD_SET(iSockFd, &rset);
    //    if(select(iSockFd + 1, &rset, NULL, NULL, &stTimeout) <= 0)
    //    {
    //        SYS_SOCKET_INFO("wait accept client connect failed, err:%s\n", strerror(errno));
    //        return ERROR;
    //	}
    //}

	int sock = accept(m_sockfd, (struct sockaddr *)&cliaddr, &clilen);
	if (sock < 0 )
	{
		return;
	}
	Infra::Debug("NetTerminal", "port:%d connect:%s:%d\n", m_port, (char*)inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
	if (m_proc.isEmpty() || !m_proc(sock, &cliaddr))
	{
		close(sock);
	}
}

/**
* @brief INetServer构造函数，权限设定protected，禁止实例
**/
INetServer::INetServer()
{

}

/**
* @brief INetServer析造函数，权限设定protected，禁止实例
**/
INetServer::~INetServer()
{

}

/**
* @brief 服务器创建函数
* @param port 服务器绑定的端口
* @param type 服务器类型
* @return 服务器接口
**/
INetServer* INetServer::create(unsigned int port, Type_t type)
{
	if (type == emTCPServer)
	{
		return CTcpServer::getServer(port);
	}
	return NULL;
}
}//NetServer
