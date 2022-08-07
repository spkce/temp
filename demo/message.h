#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <string>
#include <map>
#include "thread.h"
#include <sys/socket.h>

typedef Infra::CFunc<void, std::string> eventProc_t;

class CKeyboard
{
private:
	CKeyboard();
	~CKeyboard();
public:
	static CKeyboard* instance();
	bool init();
	bool send(const char* buf, int len);
	bool attach(std::string event, const eventProc_t & func);
private:
	void replyProc(void* arg);

private:
	const int m_RecvLen;
	int m_sockfd;
	Infra::CThread m_recvThread;
	char* m_pRecvbuf;
	std::map<std::string, eventProc_t> m_mapProc;
};
#endif
