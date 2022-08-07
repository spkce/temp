
#include "stdio.h"
#include <unistd.h>
#include "stdio.h"
#include <string.h>
#include <algorithm>
#include "Log.h"
#include "ctime.h"
#include "NetServer.h"
#include "Session.h"
#include "message.h"



using namespace NetFwk;

class CKBTermial
{
public:
	CKBTermial(CKBMessage* const pInst);
	virtual ~CKBTermial();
	virtual bool init();
	virtual bool notify(const char* buf, int len);
	bool serverTask(int sockfd, struct sockaddr_in* addr);
	void sessionTask(NetFwk::ISession* session, char* buf, int len);
#define PORT_MAIN 7800
#define MAX_SESSION 5
	const int m_maxSession;
	const int m_portMain;
	NetFwk::INetServer* m_pServ;
	NetFwk::ISession* m_session;
 	CKBMessage * const m_mssage;
};

CKBTermial::CKBTermial(CKBMessage * const pInst)
:m_maxSession(MAX_SESSION)
,m_portMain(PORT_MAIN)
,m_pServ(NULL)
,m_session(NULL)
,m_mssage(pInst)
{
}

/**
* @brief 析构函数
**/
CKBTermial::~CKBTermial()
{
	m_pServ->stop();
}

bool CKBTermial::init()
{
	m_pServ = INetServer::create(m_portMain, INetServer::emTCPServer);
	m_pServ->attach(INetServer::ServerProc_t(&CKBTermial::serverTask, this));
	m_pServ->start(m_maxSession);
	return true;
}

bool CKBTermial::notify(const char* buf, int len)
{
	if (m_session != nullptr)
	{
		return false;
	}
	m_session->send(buf, len);
	return true;
}


bool CKBTermial::serverTask(int sockfd, struct sockaddr_in* addr)
{
	if (m_session != nullptr)
	{
		return false;
	}

	ISession* pSession = CSessionManager::instance()->createSession(sockfd, addr, -1);

	if (!pSession->bind(ISession::SessionProc_t(&CKBTermial::sessionTask, this)))
	{
		CSessionManager::instance()->cancelSession(pSession);
	}
	
	return true;
}

void CKBTermial::sessionTask(ISession* session, char* buf, int len)
{
	if (session == NULL || buf == NULL || len <= 0)
	{
		return ;
	}
	m_session = session;
	session->login();
	std::string str = buf;
	m_mssage->recv(str);
}


CKBMessage::CKBMessage()
{
	m_pTermial = new CKBTermial(this);
}
CKBMessage::~CKBMessage()
{
	delete m_pTermial;
}

CKBMessage* CKBMessage::instance()
{
	static CKBMessage inst;
	return &inst;
}

bool CKBMessage::init()
{
	return m_pTermial->init();
}

void CKBMessage::recv(std::string str)
{
	auto it = m_mapProc.find(str);
	if(it != m_mapProc.end())
	{
		eventProc_t func = it->second;
		func(str);
		return ;
	}

	it = m_mapProc.find("all");
	if(it != m_mapProc.end())
	{
		eventProc_t func = it->second;
		func(str);
		return ;
	}

}

bool CKBMessage::attach(std::string event, const eventProc_t & func)
{
	if(m_mapProc.find(event) != m_mapProc.end())
	{
		return false;
	}
	m_mapProc[event] = func;
}

bool CKBMessage::send(std::string str)
{
	return m_pTermial->notify(str.c_str(), str.length());
}