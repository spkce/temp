#include <string.h>
#include "infra/ctime.h"
#include "message.h"

CMessage::CMessage(const char* s, const char* r)
:m_qs(s, 20, sizeof(struct keyMsgPacket))
,m_qr(r, 20, sizeof(struct keyMsgPacket))
{
	m_recvThread.attachProc(Infra::ThreadProc_t(&CMessage::proc, this));
	m_recvThread.createTread();
}

CMessage::~CMessage()
{
	m_recvThread.stop(true);
	m_recvThread.detachProc(Infra::ThreadProc_t(&CMessage::proc, this));
	m_mapProc.clear();
}

 CMessage* CMessage::instance()
{
	static CMessage inst("keymsgr", "keymsgs");
	return &inst;
}

bool CMessage::sendMsg(const std::string & str)
{
	struct keyMsgPacket packet = {0};
	const size_t len = str.length();
	packet.len = len < MAX_LEN - 1 ? len : MAX_LEN - 1;
	memcpy(packet.buf, str.c_str(), packet.len);

	return m_qs.input((const char*)&packet, sizeof(struct keyMsgPacket), 1);
}

bool CMessage::bind(std::string msg, const EventProc_t & fun)
{
	if (m_mapProc.find(msg) != m_mapProc.end())
	{
		return false;
	}

	m_mapProc[msg] = fun;
	return true;
}

void CMessage::proc(void* arg)
{
	std::string str;
	if (recvMsg(str))
	{
		auto it = m_mapProc.find(str);
		if (it != m_mapProc.end())
		{
			EventProc_t func = it->second;
			func(str);
			return ;
		}

		it = m_mapProc.find("all");
		if (it != m_mapProc.end())
		{
			EventProc_t func = it->second;
			func(str);
		}
		return ;
	}
	//若无事件处理，延时100ms
	Infra::CTime::delay_ms(100);
}

bool CMessage::recvMsg(std::string & str)
{
	struct keyMsgPacket packet = {0};
	if (m_qr.output((char *)&packet, sizeof(struct keyMsgPacket), 100) > 0)
	{
		str = packet.buf;
		return true;
	}
	return false;
}
