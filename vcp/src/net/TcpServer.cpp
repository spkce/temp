#include <stdio.h>
#include <unistd.h>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "string.h"
#include "TcpServer.h"
#include "Log.h"


namespace NetFwk
{

class CTcpServer : public INetServer
{
	friend class INetServer;
private:
	CTcpServer(unsigned int port);
	virtual ~CTcpServer();
	static CTcpServer* getServer(unsigned int port);
	static bool closeServer(unsigned int port);

public:
	virtual bool attach(const INetServer::ServerProc_t& proc);
	virtual bool start(unsigned int maxlisten);
	virtual bool stop();
	virtual bool isRun();

private:
	void server_task(void* arg);

	INetServer::ServerProc_t m_proc; //服务器回调函数,由attach注册
	Infra::CThread* m_pThread; //服务器线程
	int m_sockfd; //套接字句柄
	int m_port;	//服务器端口号
	
	Infra::CMutex m_mutex;

	static Infra::CMutex sm_mutex; //静态成员 互斥锁，用于保护sm_mapServer
	static std::map<unsigned int, CTcpServer*> sm_mapServer; //静态成员sm_mapServer 管理所有CTcpServer服务器实例
};

Infra::CMutex CTcpServer::sm_mutex;
std::map<unsigned int, CTcpServer*> CTcpServer::sm_mapServer;

CTcpServer::CTcpServer(unsigned int port)
:m_sockfd(-1)
,m_port(port)
{
	m_pThread = new Infra::CThread();
	m_pThread->attachProc(Infra::ThreadProc_t(&CTcpServer::server_task, this));
	m_pThread->createTread();
	
}

CTcpServer::~CTcpServer()
{
	stop();
	m_pThread->detachProc(Infra::ThreadProc_t(&CTcpServer::server_task, this));

	delete m_pThread;
	m_pThread = NULL;
}

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

bool CTcpServer::attach(const INetServer::ServerProc_t & proc)
{
	if (!m_proc.isEmpty())
	{
		return false;
	}
	m_proc = proc;
	return true;
}

bool CTcpServer::start(unsigned int maxlisten)
{
	if (m_sockfd >= 0)
	{
		Infra::Error("netFwk", "Server is already start!\n");
		return false;
	}

	m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sockfd < 0)
	{
		Infra::Error("netFwk", "open socket fail!\n");
		return false;
	}
	struct sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(m_port);
	if (bind(m_sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
	{
		Infra::Error("netFwk", "socket bind port:%d fail!\n", m_port);
		return false;
	}

	struct timeval timeout={3,0};    //设置超时时间为3秒
	setsockopt(m_sockfd, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

	if(listen(m_sockfd, maxlisten) < 0)
	{
		Infra::Error("netFwk", "socket listen port: %d fail!\n", m_port);
		return false;
	}
	m_pThread->run();
	Infra::Debug("netFwk", "tcp server: %d is ready!\n", m_port);
	return true;
}

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

bool CTcpServer::isRun()
{
	return m_sockfd >= 0;
}

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
	Infra::Debug("netFwk", "port:%d connect:%s:%d\n", m_port, (char*)inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
	if (m_proc.isEmpty() || !m_proc(sock, &cliaddr))
	{
		close(sock);
	}
}

INetServer::INetServer()
{

}

INetServer::~INetServer()
{

}

INetServer* INetServer::create(unsigned int port, Type_t type)
{
	if (type == emTCPServer)
	{
		return CTcpServer::getServer(port);
	}
	return NULL;
}

} //NetFwk
