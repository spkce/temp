#ifndef __VSPACE_CMD_H__
#define __VSPACE_CMD_H__
#include "jsoncpp.h"
#include "Protocol.h"
#include "ByteBuffer.h"

class CVcpParser
{
public:
	struct stVcpHeader
	{
		unsigned int ident;
		unsigned short reserve;
		unsigned short fsc;
		unsigned int length;
	};
public:
	CVcpParser();
	virtual ~CVcpParser();

	int input(const unsigned char* buf, unsigned int len);
	int output(Json::Value & out);

private:
	bool parser(const unsigned char* buf, unsigned int len, unsigned short csum);
	bool parserHeader(const unsigned char* buf, unsigned int len, unsigned int & length, unsigned short & csum);
	short checkSum(const unsigned char* buffer, int size);

private:
	std::list<Json::Value> m_listPacket;
	Infra::CByteBuffer m_buffer;
};

class CVSpaceCmd : public NetFwk::IProtocol
{
	enum 
	{
		ErrUnkown = 0xFFFF,
		ErrNoPermission = 0xFFFE,
		ErrRuntimeOperator = 0xFFFD,
		ErrInstNoSupport = 0xFFFC,
		ErrTimeOut = 0xFFFB,
		ErrNoSupport = 0xFFFA,
		ErrFormat = 0xFFF9,
	};

	enum
	{
		MSGID_GET_INSTANCE_LIST = 101,
		MSGID_GET_INSTANCE_INFO = 102,
		MSGID_GET_CUR_INSTANCE = 103,
		MSGID_SET_ACTIVE_INSTANCE = 104,
		MSGID_GET_REG_GROUP = 105,
		MSGID_GET_REG_LIST = 106,
		MSGID_R_REG = 107,
		MSGID_W_REG = 108,
		MSGID_R_MEM = 109,
		MSGID_W_MEM = 110,
		MSGID_GET_BREAKPOINT = 201,
		MSGID_SET_BREAKPOINT = 202,
		MSGID_DEL_BREAKPOINT = 203,
		MSGID_GET_WATCHPOINT = 204,
		MSGID_SET_WATCHPOINT = 205,
		MSGID_DEL_WATCHPOINT = 206,
		MSGID_CMD_RUN = 210,
		MSGID_CMD_STOP = 211,
		MSGID_CMD_STEP = 212,
		MSGID_SUBSCRIBE = 301,
		MSGID_PUSH_NOTIFICATION = 302,
	};


	struct stVcpHeader
	{
		unsigned int ident;
		unsigned short reserve;
		unsigned short fsc;
		unsigned int length;
	};


public:
	CVSpaceCmd(NetFwk::ISession* session, size_t recvlen);
	virtual ~CVSpaceCmd();

	virtual int parse(const unsigned char* buf, int len);

private:
	bool messageControl(Json::Value &request, Json::Value &response);
	bool login(Json::Value &request, Json::Value &response);
	bool logout(Json::Value &request, Json::Value &response);
	bool keepAlive(Json::Value &request, Json::Value &response);

	bool messageHub(Json::Value &request, Json::Value &response);

	bool getInstanceList(Json::Value &request, Json::Value &response);
	bool getInstanceInfo(Json::Value &request, Json::Value &response);
	bool getCurInstance(Json::Value &request, Json::Value &response);
	bool setActiveInstance(Json::Value &request, Json::Value &response);
	bool getRegsiterGroup(Json::Value &request, Json::Value &response);
	bool getRegsiterList(Json::Value &request, Json::Value &response);
	bool getRegsiter(Json::Value &request, Json::Value &response);
	bool setRegsiter(Json::Value &request, Json::Value &response);
	bool getMemory(Json::Value &request, Json::Value &response);
	bool setMemory(Json::Value &request, Json::Value &response);

	bool getBreakPoint(Json::Value &request, Json::Value &response);
	bool setBreakPoint(Json::Value &request, Json::Value &response);
	bool delBreakPoint(Json::Value &request, Json::Value &response);
	bool getWatchPoint(Json::Value &request, Json::Value &response);
	bool setWatchPoint(Json::Value &request, Json::Value &response);
	bool delWatchPoint(Json::Value &request, Json::Value &response);

	bool cmdRun(Json::Value &request, Json::Value &response);
	bool cmdStop(Json::Value &request, Json::Value &response);
	bool cmdStep(Json::Value &request, Json::Value &response);
private:
	void sendPacket(Json::Value & packet);
	unsigned short checkSum(const unsigned char* buffer, int size);

private:
	long m_sessionId;
	CVcpParser m_parser;
};

#endif //__VSPACE_CMD_H__
