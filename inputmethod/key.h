
#include <map>
#include <QToolButton>
#include "infra/thread.h"
#include "infra/MsgQueue.h"

class Key:public QToolButton
{
    Q_OBJECT
public:
    Key(QString name,int w,int h);
    ~Key();

private slots:
	void onclick();
};

class CMessage
{
	#define MAX_LEN 20
public:
	struct keyMsgPacket
	{
		char buf[MAX_LEN];
		size_t len;
	};
	typedef Infra::CFunc<void, std::string> EventProc_t;
public:
	static CMessage* instance();
	bool sendMsg(const std::string & str);
	bool bind(std::string msg, const EventProc_t & fun);

private:
	CMessage(const char* s, const char* r);
	virtual ~CMessage();
	void proc(void* arg);
	bool recvMsg(std::string & str);

private: 
	Infra::CMsgQueue m_qs;
	Infra::CMsgQueue m_qr;
	Infra::CThread m_recvThread;
	std::map<std::string, EventProc_t> m_mapProc;
};
