#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Session.h"
#include "thread.h"
#include "ctime.h"
#include "Log.h"

namespace NetFwk
{

class CSession : public ISession
{
public:
	CSession();

protected:
	virtual ~CSession();

public:
	void set(int sockfd, struct sockaddr_in* addr, int timeout);
	virtual bool login();
	virtual bool logout();
	virtual bool keepAlive();
	virtual int getTimeout() const;
	virtual state_t getState() const;
	virtual bool close();
	
	virtual void destroy();
	virtual int recv(char* buf, int len);
	virtual int send(const char* buf, int len);

private:
	void timerProc(unsigned long long arg);

private:

	struct sockaddr_in m_addr;
	Infra::CTimer m_timer;

	unsigned long long m_lastTime;
	int m_timeout;
	int m_sockfd;
	state_t m_state;
};

CSession::CSession()
:m_lastTime(0)
,m_timeout(-1)
,m_sockfd(-1)
,m_state(emStateCreated)
{
	memset(&m_addr, 0, sizeof(struct sockaddr_in));
}

CSession::~CSession()
{
	close();
}

void CSession::set(int sockfd, struct sockaddr_in* addr, int timeout)
{
	m_sockfd = sockfd;
	memcpy(&m_addr, addr, sizeof(struct sockaddr_in));
	m_timeout = timeout;

	struct timeval _timeout = {3,0}; //默认3秒超时
	if (timeout != -1)
	{
		_timeout.tv_sec = timeout;
	}
	else
	{
		m_state = emStateLogin;
	}

	if (setsockopt(m_sockfd, SOL_SOCKET, SO_RCVTIMEO,(char*)&_timeout,sizeof(struct timeval)) == -1)
	{
		Infra::Error("netFwk","setsockopt error : %s\n", strerror(errno));
		return;
	}
	
	if (setsockopt(m_sockfd, SOL_SOCKET, SO_SNDTIMEO,(char*)&_timeout,sizeof(struct timeval)) == -1)
	{
		Infra::Error("netFwk","setsockopt error : %s\n", strerror(errno));
		return;
	}

	if (timeout > 0)
	{
		m_timer.setTimerAttr(Infra::CTimer::TimerProc_t(&CSession::timerProc, this), 3000);
		m_timer.run();
	}

}

bool CSession::login()
{
	if (m_state < emStateLogin)
	{
		m_lastTime = Infra::CTime::getSystemTimeSecond();
		m_state = emStateLogin;
		return true;
	}
	return false;
}

bool CSession::logout()
{
	if (m_state == emStateLogin)
	{
		m_state = emStateLogout;
		Infra::Debug("netFwk", "Session:%s:%d logout\n", (char*)inet_ntoa(m_addr.sin_addr), ntohs(m_addr.sin_port));
		return true;
	}
	return false;
}

bool CSession::keepAlive()
{
	if (m_state == emStateLogin)
	{
		m_lastTime = Infra::CTime::getSystemTimeSecond();
		return true;
	}
	return false;
}

int CSession::getTimeout() const
{
	return m_timeout;
}

ISession::state_t CSession::getState() const
{
	return m_state;
}

bool CSession::close()
{
	if (m_timeout > 0 && m_timer.isRun())
	{
		m_timer.stop();
	}

	switch (m_state)
	{
	case emStateLogin:
		logout();
	case emStateLogout:
	case emStateCreated:
		if (m_sockfd >= 0)
		{
			::close(m_sockfd);
			m_sockfd = -1;
		}
		m_state = emStateClose;
	case emStateClose:
		return true;
	default:
		return false;
	}
}

void CSession::destroy()
{
	Infra::Debug("netFwk", "Session:%s:%d destory\n", (char*)inet_ntoa(m_addr.sin_addr), ntohs(m_addr.sin_port));
	delete this;
}

int CSession::send(const char* buf, int len)
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

	Infra::Debug("netFwk", "send len : %d -> %s:%d\n", len, (char*)inet_ntoa(m_addr.sin_addr), ntohs(m_addr.sin_port));

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
			Infra::Error("netFwk","send err : %s\n", strerror(errno));
			if (m_timeout == -1)
			{
				//无超时的session，发送失败便登出
				close();
			}

			return -1;
		}
	}

	return len;
}

int CSession::recv(char* buf, int len)
{
	if (buf == NULL || len <= 0)
	{
		return -1;
	}

	int rlen = ::recv(m_sockfd, buf, len, 0);
	if (rlen > 0)
	{
		Infra::Debug("netFwk","recv:%s:%d len=%d\n", (char*)inet_ntoa(m_addr.sin_addr), ntohs(m_addr.sin_port), rlen);
	}

	return rlen;
}

void CSession::timerProc(unsigned long long arg)
{
	if ((m_lastTime + m_timeout) <= Infra::CTime::getSystemTimeSecond())
	{
		logout();
	}
}

ISession* ISession::create(int sockfd, struct sockaddr_in* addr, int timeout)
{
	CSession* p = new CSession();
	p->set(sockfd, addr, timeout);
	return p;
}

} //NetFwk
