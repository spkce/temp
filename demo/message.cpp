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


void CKeyboard::replyProc(void* arg)
{
	memset(m_pRecvbuf, 0, m_RecvLen);
	int len = recv(m_sockfd, m_pRecvbuf, m_RecvLen, 0);

	if (len <= 0)
	{
		return;
	}

	std::string str = m_pRecvbuf;
	printf("recv:%s", str.c_str());
}
