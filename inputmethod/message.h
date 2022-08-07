#ifndef __MESSAGE_H__
#define __MESSAGE_H__
#include <map>
#include <string>
#include "Infra/thread.h"


class CKBTermial;

typedef Infra::CFunc<void, std::string> eventProc_t;

class CKBMessage
{
	friend CKBTermial;
private:
	CKBMessage();
	~CKBMessage();
public:
	static CKBMessage* instance();
	bool init();
	bool attach(std::string event, const eventProc_t & func);
	bool send(std::string str);
private:
	void recv(std::string str);
	
private:
	CKBTermial *m_pTermial;
	std::map<std::string, eventProc_t> m_mapProc;
};
#endif
