#include "Log.h"
#include "ctime.h"
#include "terminal.h"
#include "Session.h"
#include "TcpServer.h"

namespace NetFwk
{

ITerminal::ITerminal(unsigned int max, unsigned int recvLen)
:m_pServer(nullptr)
,m_maxSession(max)
,m_recvLen(recvLen)
,m_port(0)
,m_timeout(-1)
{
	m_thread.attachProc(Infra::ThreadProc_t(&ITerminal::managerTask, this));
	m_thread.createTread();
}

ITerminal::~ITerminal()
{
	stop();
	m_thread.detachProc(Infra::ThreadProc_t(&ITerminal::managerTask, this));
}

bool ITerminal::init(unsigned int port, int timeout)
{
	if (m_port != 0)
	{
		return false;
	}

	m_pServer = INetServer::create(port, INetServer::emTCPServer);
	m_pServer->attach(INetServer::ServerProc_t(&ITerminal::serverTask, this));
	if (!m_pServer->start(m_maxSession))
	{
		return false;
	}

	m_timeout = timeout;
	m_port = port;

	m_thread.run();
	return true;
}

bool ITerminal::stop()
{
	m_thread.stop();

	if (m_pServer->isRun())
	{
		m_pServer->stop();
	}

	Infra::CGuard<Infra::CMutex> guard(m_mutex);
	for (auto it = m_listProtocol.begin(); it != m_listProtocol.end(); ++it)
	{
		IProtocol* p = *it;
		delete p;
	}

	m_listProtocol.clear();
	return true;
}

bool ITerminal::serverTask(int sockfd, struct sockaddr_in* addr)
{
	if (m_listProtocol.size() > m_maxSession)
	{
		Infra::Warning("netFwk", "the number of session reaches its maximun !\n");
		return false;
	}

	ISession* pSession = ISession::create(sockfd, addr, m_timeout);
	if (pSession == nullptr)
	{
		Infra::Warning("netFwk", "Session create fail!\n");
		return false;
	}

	IProtocol* p = createProtocol(pSession, m_recvLen);
	p->start();
	if (p == nullptr)
	{
		Infra::Warning("netFwk", "Procotol create fail!\n");
		return false;
	}

	Infra::CGuard<Infra::CMutex> guard(m_mutex);
	m_listProtocol.push_back(p);
	return true;
}

void ITerminal::managerTask(void* arg)
{
	m_mutex.lock();
	for (auto it = m_listProtocol.begin(); it != m_listProtocol.end();)
	{
		IProtocol* p = *it;
		if (p->isLogout())
		{
			m_listProtocol.erase(it++);
			delete p;
		}
		else
		{
			it++;
		}
	}
	m_mutex.unlock();

	Infra::CTime::delay_ms(1000);
}

} // NetFwk
