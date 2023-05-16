#include <string.h>
#include <map>
#include "thread.h"
#include "Protocol.h"

namespace NetFwk
{

IProtocol::IProtocol(ISession* session, size_t recvLen)
:m_timer()
,m_session(session)
,m_recvLen(recvLen)
{

	m_pBuffer = new unsigned char [recvLen];

	m_thread.attachProc(Infra::ThreadProc_t(&IProtocol::sessionTask, this));
	m_thread.createTread();
}

IProtocol::~IProtocol()
{
	m_session->close();
	m_thread.stop(true);
	m_thread.detachProc(Infra::ThreadProc_t(&IProtocol::sessionTask, this));


	m_session->destroy();
	delete [] m_pBuffer;
}

bool IProtocol::start()
{
	m_thread.run();
	return true;
}

int IProtocol::send(const char* buf, int len)
{
	if (m_session->getState() < ISession::emStateClose)
	{
		return m_session->send(buf, len);
	}
	return -1;
}

bool IProtocol::login()
{
	return m_session->login();
}

bool IProtocol::logout()
{
	return m_session->logout();
}

bool IProtocol::keepAlive()
{
	return m_session->keepAlive();
}

bool IProtocol::isLogout() const
{
	return m_session->getState() == ISession::emStateLogout;
}

void IProtocol::sessionTask(void* arg)
{
	if (m_session->getState() < ISession::emStateClose)
	{
		size_t recvLen = m_recvLen;

		while (recvLen)
		{
			recvLen = recvLen > m_recvLen ? m_recvLen : recvLen;
			memset(m_pBuffer, 0, recvLen);
			int len = m_session->recv((char*)m_pBuffer, recvLen);
			if (len <= 0 || m_session->getState() == ISession::emStateClose)
			{
				return ;
			}

			recvLen = (size_t)parse(m_pBuffer, len);
		}
	}
}
} //NetFwk
