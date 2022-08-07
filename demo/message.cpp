#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "thread.h"
#include "ctime.h"
#include "Log.h"
#include "message.h"


CKeyboard::CKeyboard()
:m_RecvLen(1024)
{
	m_recvThread.attachProc(Infra::ThreadProc_t(&CKeyboard::replyProc, this));
	m_pRecvbuf = new char[m_RecvLen];
}
CKeyboard::~CKeyboard()
{
	m_recvThread.detachProc(Infra::ThreadProc_t(&CKeyboard::replyProc, this));
	delete m_pRecvbuf;
}

CKeyboard* CKeyboard::instance()
{
	static CKeyboard inst;
	return &inst;
}
bool CKeyboard::init()
{
	m_sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(m_sockfd < 0)
	{
		return false;
	}

	struct timeval _timeout = {3,0};
	if (setsockopt(m_sockfd, SOL_SOCKET, SO_RCVTIMEO,(char*)&_timeout,sizeof(struct timeval)) == -1)
	{
		Infra::Error("NetTerminal","setsockopt error : %s\n", strerror(errno));
		return false;
	}
	
	if (setsockopt(m_sockfd, SOL_SOCKET, SO_SNDTIMEO,(char*)&_timeout,sizeof(struct timeval)) == -1)
	{
		Infra::Error("NetTerminal","setsockopt error : %s\n", strerror(errno));
		return false;
	}

	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(7800);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(m_sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		return false;
	}

	m_recvThread.createTread();
	m_recvThread.run();
	return true;
}

bool CKeyboard::send(const char* buf, int len)
{
	if (m_sockfd < 0)
	{
		return m_sockfd;
	}

	if (buf == NULL || len <= 0)
	{
		return -1;
	}

	char* p = (char*)buf;
	while (len > 0)
	{
		int sendlen = ::send(m_sockfd, p, len, 0);
		if (sendlen > 0 )
		{
			len -= sendlen;
			p += sendlen;
		}
		else if (errno == EINTR)
		{
			continue;
		}
		else
		{
			Infra::Error("NetTerminal","send err : %s\n", strerror(errno));

			return -1;
		}
	}

	return len;
}

bool CKeyboard::attach(std::string event, const eventProc_t & func)
{
	if(m_mapProc.find(event) != m_mapProc.end())
	{
		return false;
	}
	m_mapProc[event] = func;
	return true;
}

void CKeyboard::replyProc(void* arg)
{
	Infra::Error("NetTerminal","replyProc\n");

	memset(m_pRecvbuf, 0, m_RecvLen);
	int len = recv(m_sockfd, m_pRecvbuf, m_RecvLen, 0);

	if (len <= 0)
	{
		Infra::CTime::delay_ms(500);
		return;
	}

	std::string str = m_pRecvbuf;
	Infra::Error("NetTerminal","find str:%s\n", str.c_str());
	
	auto it = m_mapProc.find(str);
	if(it != m_mapProc.end())
	{
		Infra::Error("NetTerminal","find str:%s\n", str.c_str());
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
