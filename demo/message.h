#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <string>
#include "thread.h"
#include <sys/socket.h>

class CKeyboard
{
private:
	CKeyboard();
	~CKeyboard();
public:
	static CKeyboard* instance();
	bool init();

private:
	void replyProc(void* arg);

private:
	const int m_RecvLen;
	int m_sockfd;
	Infra::CThread m_recvThread;
	char* m_pRecvbuf;
};
#endif
