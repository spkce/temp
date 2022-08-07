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

/**
* @brief 会话类
**/
class CSession : public ISession
{
	//friend class CSessionManager;
private:
	#define MAX_DATA_BUF 4096
 	struct sendPacket
 	{
		int len;
		char* buf[MAX_DATA_BUF];
 	};
protected: 
	/**
	* @brief 构造函数， 设定protected权限，不可实例，可继承
	**/
	CSession();

	/**
	* @brief 析构函数， 设定protected权限，不可实例，可继承
	**/
	virtual ~CSession();

public:
	/**
	* @brief 会话设定
	* @param sockfd 套接字句柄
	* @param addr 对端地址
	* @param timeout 超时时间
	**/
	void set(int sockfd, struct sockaddr_in* addr, int timeout);

	/**
	* @brief 会话回调函数绑定
	* @param proc 回调函数
	* @return 成功：true；失败：false
	**/
	virtual bool bind(const ISession::SessionProc_t & proc);

	/**
	* @brief 会话回调函数取消绑定
	* @return 成功：true；失败：false
	**/
	virtual bool unbind();

	/**
	* @brief 登录
	* @return 成功：true；失败：false
	**/
	virtual bool login();

	/**
	* @brief 登出
	* @return 成功：true；失败：false
	**/
	virtual bool logout();

	/**
	* @brief 保活
	* @return 成功：true；失败：false
	**/
	virtual bool keepAlive();

	/**
	* @brief 是否超时
	* @return 超时：true；未超时：false
	**/
	virtual bool isTimeout();

	/**
	* @brief 获取会话状态
	* @return 获取会话状态
	**/
	virtual state_t getState();

	/**
	* @brief 关闭会话
	* @return 成功：true；失败：false
	**/
	virtual bool close();

	/**
	* @brief 创建会话
	* @return 会话指针
	**/
	static CSession* create();

	/**
	* @brief 会话释放
	**/
	virtual void destroy();

	/**
	* @brief 发送消息，同步
	* @param buf 消息内容
	* @param len 消息长度
	* @return 发送的数据长度
	**/
	virtual int send(const char* buf, int len);

	/**
	* @brief 接收回调函数
	* @param arg 无用参数
	**/
	void replyProc(void* arg);

	/**
	* @brief 定时器回调函数
	* @param arg 系统运行时间 ms
	**/
	void timerProc(unsigned long long arg);

private:

#define RECV_LEN 1024
	Infra::CThread m_recvThread;
	struct sockaddr_in m_addr;
	ISession::SessionProc_t m_proc;
	Infra::CTimer m_timer;
	const int m_RecvLen;

	unsigned long long m_lastTime;
	int m_timeout;
	int m_sockfd;
	state_t m_state;
	char* m_pRecvbuf;
};

/**
* @brief 构造函数， 设定protected权限，不可实例，可继承
**/
CSession::CSession()
:m_RecvLen(RECV_LEN)
,m_lastTime(0)
,m_timeout(-1)
,m_sockfd(-1)
,m_state(emStateCreated)
{
	memset(&m_addr, 0, sizeof(struct sockaddr_in));
	m_recvThread.attachProc(Infra::ThreadProc_t(&CSession::replyProc, this));
	m_pRecvbuf = new char[m_RecvLen];

	m_timer.setTimerAttr(Infra::CTimer::TimerProc_t(&CSession::timerProc, this), 3000);
}

/**
* @brief 析构函数， 设定protected权限，不可实例，可继承
**/
CSession::~CSession()
{
	if (m_timer.isRun())
	{
		m_timer.stop();
	}

	m_recvThread.detachProc(Infra::ThreadProc_t(&CSession::replyProc, this));
	delete[] m_pRecvbuf;
}

/**
* @brief 会话设定
* @param sockfd 套接字句柄
* @param addr 对端地址
* @param timeout 超时时间
**/
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

	if (setsockopt(m_sockfd, SOL_SOCKET, SO_RCVTIMEO,(char*)&_timeout,sizeof(struct timeval)) == -1)
	{
		Infra::Error("NetTerminal","setsockopt error : %s\n", strerror(errno));
		return;
	}
	
	if (setsockopt(m_sockfd, SOL_SOCKET, SO_SNDTIMEO,(char*)&_timeout,sizeof(struct timeval)) == -1)
	{
		Infra::Error("NetTerminal","setsockopt error : %s\n", strerror(errno));
		return;
	}
}

/**
* @brief 会话回调函数绑定
* @param proc 回调函数
* @return 成功：true；失败：false
**/
bool CSession::bind(const ISession::SessionProc_t & proc)
{
	if (!m_proc.isEmpty())
	{
		return false;
	}
	m_proc = proc;
	m_recvThread.createTread();
	m_recvThread.run();
	m_state = emStateLogout;
	m_lastTime = Infra::CTime::getSystemTimeSecond();
	if (m_timeout > 0 && !m_timer.isRun())
	{
		//当超时时间大于0时才有必要启动定时器
		m_timer.run();
	}
	return true;
}

/**
* @brief 会话回调函数取消绑定
* @return 成功：true；失败：false
**/
bool CSession::unbind()
{
	if (m_proc.isEmpty())
	{
		return false;
	}
	m_recvThread.stop(true);
	m_proc.unbind();
	
	if (m_timeout > 0 && m_timer.isRun())
	{
		m_timer.stop();
	}
	m_state = emStateClose;
	return true;
}

/**
* @brief 登录
* @return 成功：true；失败：false
**/
bool CSession::login()
{
	if (m_state == emStateLogout)
	{
		m_lastTime = Infra::CTime::getSystemTimeSecond();
		m_state = emStateLogin;
		return true;
	}
	return false;
}

/**
* @brief 登出
* @return 成功：true；失败：false
**/
bool CSession::logout()
{
	if (m_state == emStateLogin)
	{
		m_state = emStateLogout;
		return true;
	}
	return false;
}

/**
* @brief 保活
* @return 成功：true；失败：false
**/
bool CSession::keepAlive()
{
	if (m_state == emStateLogin)
	{
		m_lastTime = Infra::CTime::getSystemTimeSecond();
		return true;
	}
	return false;
}

/**
* @brief 是否超时
* @return 超时：true；未超时：false
**/
bool CSession::isTimeout()
{
	if (m_timeout >= 0 && (m_lastTime + m_timeout) <= Infra::CTime::getSystemTimeSecond())
	{
		return true;
	}

	return false;
}

/**
* @brief 获取会话状态
* @return 获取会话状态
**/
ISession::state_t CSession::getState()
{
	return m_state;
}

/**
* @brief 关闭会话
* @return 成功：true；失败：false
**/
bool CSession::close()
{
	switch (m_state)
	{
	case emStateLogin:
		logout();
	case emStateLogout:
		unbind();
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

/**
* @brief 创建会话
* @return 会话指针
**/
CSession* CSession::create()
{
	return new CSession();
}

/**
* @brief 会话释放
**/
void CSession::destroy()
{
	Infra::Debug("NetTerminal", "Session:%s:%d destory\n", (char*)inet_ntoa(m_addr.sin_addr), ntohs(m_addr.sin_port));
	delete this;
}

/**
* @brief 发送消息，同步
* @param buf 消息内容
* @param len 消息长度
* @return 发送的数据长度
**/
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

	Infra::Debug("NetTerminal", "send len : %d -> %s:%d\n", len, (char*)inet_ntoa(m_addr.sin_addr), ntohs(m_addr.sin_port));

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


/**
* @brief 接收回调函数
* @param arg 无用参数
**/
void CSession::replyProc(void* arg)
{
	memset(m_pRecvbuf, 0, m_RecvLen);
	int len = recv(m_sockfd, m_pRecvbuf, m_RecvLen, 0);

	if (len <= 0 || m_proc.isEmpty())
	{
		return;
	}

	Infra::Debug("NetTerminal","recv:%s:%d len=%d\n", (char*)inet_ntoa(m_addr.sin_addr), ntohs(m_addr.sin_port), len);
	m_proc(this, m_pRecvbuf, len);
	
}

/**
* @brief 定时器回调函数
* @param arg 系统运行时间 ms
**/
void CSession::timerProc(unsigned long long arg)
{
	if (isTimeout())
	{
		Infra::Debug("NetTerminal", "Session:%s:%d time out\n", (char*)inet_ntoa(m_addr.sin_addr), ntohs(m_addr.sin_port));
		close();
	}
}

/**
* @brief 构造函数， 设定private权限，外部不可实例，不可继承
**/
CSessionManager::CSessionManager()
:m_timer("SessionManager")
{
	m_timer.setTimerAttr(Infra::CTimer::TimerProc_t(&CSessionManager::timerProc, this), 3000);
	m_timer.run();
}

/**
* @brief 析构函数， 设定private权限，外部不可实例，不可继承
**/
CSessionManager::~CSessionManager()
{
	if (m_timer.isRun())
	{
		m_timer.stop();
	}
}

/**
* @brief 获取会话管理类实例
**/
CSessionManager* CSessionManager::instance()
{
	static CSessionManager* pInstance = NULL;
	if (pInstance == NULL)
	{
		static Infra::CMutex sm_mutex;
		Infra::CGuard<Infra::CMutex> guard(sm_mutex);
		if (pInstance == NULL)
		{
			pInstance = new CSessionManager;
		}
	}
	return pInstance;
}

/**
* @brief 创建session
* @param sockfd 套接字句柄
* @param addr 对端地址
* @param timeout 超时时间单位秒，默认值-1 不会超时
* @return 会话基类指针
**/
ISession* CSessionManager::createSession(int sockfd, struct sockaddr_in* addr, int timeout)
{
	if (sockfd < 0 || addr == NULL)
	{
		return NULL;
	}
	CSession* pSession = CSession::create();
	
	pSession->set(sockfd, addr, timeout);
	
	registerSession(pSession);
	return pSession;
}

/**
* @brief 查询session是否注册中
* @param session 指针
* @return 是否注册
**/
bool CSessionManager::isSessionRegister(ISession* session)
{
	if (session == NULL)
	{
		return false;
	}

	Infra::CGuard<Infra::CMutex> guard(m_mutex);
	std::vector<ISession*>::iterator iter = find(m_vecSession.begin(), m_vecSession.end(), session);

	return iter == m_vecSession.end();
}

/**
* @brief 关闭session
* @param session 会话基类指针
**/
bool CSessionManager::cancelSession(ISession* session)
{
	if (session == NULL)
	{
		return false;
	}

	Infra::CGuard<Infra::CMutex> guard(m_mutex);

	std::vector<ISession*>::iterator iter = find(m_vecSession.begin(), m_vecSession.end(), session);

	if (iter != m_vecSession.end())
	{
		session->close();
		return true;
	}
	else
	{
		return false;
	}
}

/**
* @brief 查询session状态
* @param session 指针
* @return 状态
**/
int CSessionManager::getState(ISession* session)
{
	if (session == NULL)
	{
		return ISession::emStateNone;
	}

	Infra::CGuard<Infra::CMutex> guard(m_mutex);

	std::vector<ISession*>::iterator iter = find(m_vecSession.begin(), m_vecSession.end(), session);

	if (iter != m_vecSession.end())
	{
		return session->getState();
	}
	else
	{
		return ISession::emStateNone;
	}
}

/**
* @brief 注册session
* @param session 会话基类指针
**/
void CSessionManager::registerSession(ISession* session)
{
	if (session == NULL)
	{
		return ;
	}

	Infra::CGuard<Infra::CMutex> guard(m_mutex);

	std::vector<ISession*>::iterator iter = find(m_vecSession.begin(), m_vecSession.end(), session);
	if (iter == m_vecSession.end())
	{
		m_vecSession.push_back(session);
	}
}

/**
* @brief 定时器回调函数
* @param arg 时间，系统运行时间毫秒
**/
void CSessionManager::timerProc(unsigned long long arg)
{
	Infra::CGuard<Infra::CMutex> guard(m_mutex);
	std::vector<ISession*>::iterator it;
	for (it = m_vecSession.begin(); it != m_vecSession.end();)
	{
		ISession* p = *it;
		if (p->getState() == ISession::emStateClose)
		{
			it = m_vecSession.erase(it);
			p->destroy(); 
		}
		else
		{
			it++;
		}
	}

}

} //NetFwk