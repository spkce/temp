#include <list>
#include "Log.h"
#include "vSpaceCmd.h"

CVcpParser::CVcpParser()
{
}

CVcpParser::~CVcpParser()
{
}

int CVcpParser::input(const unsigned char* buf, unsigned int len)
{
	while (len > 0 || m_buffer.size() > 0)
	{
		if (m_buffer.size() > 0)
		{
			if  (len > 0)
			{
				m_buffer.append(buf, len);
				len = 0;
				buf += len;
			}

			if (m_buffer.size() < sizeof(struct stVcpHeader))
			{
				return 0;
			}
			unsigned int length = 0;
			unsigned short csum = 0;
			const unsigned int size = m_buffer.size();
			const unsigned char* p = m_buffer.getBuffer();

			unsigned int i = 0;
			for (; i < size; i++)
			{
				if (parserHeader(p + i, size - i, length, csum))
				{
					if (length > size - i)
					{
						m_buffer.remove(-i);
						return length - (size - i);
					}
					
					parser(p + i, length, csum);
					m_buffer.remove(-length);
					break;
				}
			}
			m_buffer.remove(-i); // remove other byte front of header
		}
		else
		{
			unsigned int length = 0;
			unsigned short csum = 0;
			if (parserHeader(buf , len, length, csum))
			{
				if (len < length)
				{
					m_buffer.append(buf, len);
					len = 0;
					buf += len;
					continue;
				}

				parser(buf, length, csum);
				len -= length;
				buf += length;

				if (len < sizeof(struct stVcpHeader))
				{
					m_buffer.append(buf, len);
					len = 0;
					buf += len;
				}
			}
		}
	}

	return 0;
}

int CVcpParser::output(Json::Value & out)
{
	if (m_listPacket.empty())
	{
		return -1;
	}

	auto it = m_listPacket.begin();
	out.swap(*it);
	m_listPacket.pop_front();

	return (int)m_listPacket.size();
}

bool CVcpParser::parser(const unsigned char* buf, unsigned int len, unsigned short csum)
{
	if (len < sizeof(struct stVcpHeader))
	{
		Infra::Error("CmdServer", "Length too small!\n");
	}

	const unsigned char * payload = buf + sizeof(struct stVcpHeader);
	unsigned short fsc = checkSum((const unsigned char*)payload , len - sizeof(struct stVcpHeader));
	if (csum != fsc)
	{
		Infra::Error("CmdServer", "Wrong FSC:%d %d\n", csum, fsc);
		return false;
	}

	std::string recv = (const char*)payload;
	Json::String errs;
	Json::Value request;
	Json::CharReaderBuilder readerBuilder;
	std::unique_ptr<Json::CharReader> const jsonReader(readerBuilder.newCharReader());
	bool res = jsonReader->parse(recv.c_str(), recv.c_str() + recv.length(), &request, &errs);
	if (!res || !errs.empty())
	{
		Infra::Error("CmdServer", "json parse err:%s\n", errs.c_str());
		return false;
	}

	m_listPacket.push_back(request);
	return true;
}

bool CVcpParser::parserHeader(const unsigned char* buf, unsigned int len, unsigned int & length, unsigned short & csum)
{
	if (!buf || len < sizeof(struct stVcpHeader))
	{
		Infra::Error("CmdServer", "invalid input parameter\n");
		return false;
	}

	const struct stVcpHeader * pVcpHdr = (const struct stVcpHeader *)buf;
	if (pVcpHdr->ident != htonl(0x7E766370))
	{
		Infra::Error("CmdServer", "Wrong identifier:%d\n", ntohl(pVcpHdr->ident));
		return false;
	}

	length = htonl(pVcpHdr->length);
	csum = ntohs(pVcpHdr->fsc);
	return true;
}

short CVcpParser::checkSum(const unsigned char* buffer, int size)
{
	unsigned long cksum = 0;
	unsigned short * pbuf = (unsigned short *)buffer;
	for (int i = 0; i < size / 2; i++)
	{
		cksum += htons(pbuf[i]);
	}
	
	if( size % 2 == 1) 
	{ 
		cksum += (unsigned short)(buffer[size - 1] << 8);
	}
	
	while (cksum >> 16) 
	{
		cksum = (cksum >> 16) + (cksum & 0xffff); 
	}
	
	return (unsigned short)(~cksum);
}

CVSpaceCmd::CVSpaceCmd(NetFwk::ISession* session, size_t recvlen)
:IProtocol(session, recvlen)
,m_sessionId((long)session)
,m_parser()
{
}

CVSpaceCmd::~CVSpaceCmd()
{
}

int CVSpaceCmd::parse(const unsigned char* buf, int len)
{
	if (len <= 0)
	{
		return 0;
	}

	int length = m_parser.input(buf, len);
	if (length > 0)
	{
		return length;
	}

	Json::Value request = Json::nullValue;
	while (m_parser.output(request) >= 0)
	{
		Infra::Info("CmdServer","request = %s\n", request.toStyledString().c_str());
		Json::Value response = Json::nullValue;

		if (request.isMember("msg") && request["msg"].isString())
		{
			messageControl(request, response);
		}
		else if (request.isMember("msgID") && request["msgID"].isUInt())
		{
			messageHub(request, response);
		}
		else
		{
			response["sessionID"] = m_sessionId;
			response["result"] = "fail";
			response["errcode"] = ErrNoSupport;

		}

		Infra::Info("CmdServer","response = %s\n", response.toStyledString().c_str());

		sendPacket(response);
	}

	return 0;
}

bool CVSpaceCmd::messageControl(Json::Value &request, Json::Value &response)
{
	std::string msg = request["msg"].asString();
	if (msg == "login")
	{
		return login(request, response);
	}
	else if (msg == "logout")
	{
		return logout(request, response);
	}
	else if (msg == "keepAlive")
	{
		return keepAlive(request, response);
	}

	return false;
}

bool CVSpaceCmd::login(Json::Value &request, Json::Value &response)
{
	response["msg"] = request["msg"];
	if (m_session->getState() != NetFwk::ISession::emStateLogin && IProtocol::login())
	{
		response["sessionID"] = m_sessionId;
		response["result"] = "success";
		return true;
	}
	else
	{
		response["result"] = "fail";
		return false;
	}
}

bool CVSpaceCmd::logout(Json::Value &request, Json::Value &response)
{
	response["msg"] = request["msg"];
	response["sessionID"] = m_sessionId;
	if (m_session->getState() == NetFwk::ISession::emStateLogin && IProtocol::logout())
	{
		response["result"] = "success";
		return true;
	}
	else
	{
		response["result"] = "fail";
		return false;
	}
}

bool CVSpaceCmd::keepAlive(Json::Value &request, Json::Value &response)
{
	response["msg"] = request["msg"];
	response["sessionID"] = m_sessionId;
	if (m_session->getState() == NetFwk::ISession::emStateLogin && IProtocol::keepAlive())
	{
		response["result"] = "success";
		return true;
	}
	else
	{
		response["result"] = "fail";
		return false;
	}
}

bool CVSpaceCmd::messageHub(Json::Value &request, Json::Value &response)
{
	unsigned int msgID = request["msgID"].asUInt();
	
	response["msgID"] = msgID;
	response["sessionID"] = m_sessionId;
	
	switch (msgID)
	{
	case MSGID_GET_INSTANCE_LIST:
		return getInstanceList(request, response);
	
	case MSGID_GET_INSTANCE_INFO:
		return getInstanceInfo(request, response);
	case MSGID_GET_CUR_INSTANCE:
		return getCurInstance(request, response);
	case MSGID_SET_ACTIVE_INSTANCE:
		return setActiveInstance(request, response);
	case MSGID_GET_REG_GROUP:
		return getRegsiterGroup(request, response);
	case MSGID_GET_REG_LIST:
		return getRegsiterList(request, response);
	case MSGID_R_REG:
		return getRegsiter(request, response);
	case MSGID_W_REG:
		return setRegsiter(request, response);
	case MSGID_R_MEM:
		return getMemory(request, response);
	case MSGID_W_MEM:
		return setMemory(request, response);
	case MSGID_GET_BREAKPOINT:
		return getBreakPoint(request, response);
	case MSGID_SET_BREAKPOINT:
		return delBreakPoint(request, response);
	case MSGID_DEL_BREAKPOINT:
		return setWatchPoint(request, response);
	case MSGID_GET_WATCHPOINT:
		return getWatchPoint(request, response);
	case MSGID_SET_WATCHPOINT:
		return setWatchPoint(request, response);
	case MSGID_DEL_WATCHPOINT:
		return delWatchPoint(request, response);
	case MSGID_CMD_RUN:
		return cmdRun(request, response);
	case MSGID_CMD_STOP:
		return cmdStop(request, response);
	case MSGID_CMD_STEP:
		return cmdStep(request, response);
	case MSGID_SUBSCRIBE:
	case MSGID_PUSH_NOTIFICATION:
	default:
		
		response["result"] = "fail";
		response["errcode"] = ErrNoSupport;
		return true;
	}
}

bool CVSpaceCmd::getInstanceList(Json::Value &request, Json::Value &response)
{
	response["list"][0]["id"] = 1;
	response["list"][0]["name"] = "aarch64 core";
	response["list"][0]["type"] = "core";
	response["list"][0]["caps"] = "0x1f";

	return true;
}

bool CVSpaceCmd::getInstanceInfo(Json::Value &request, Json::Value &response)
{
	response["info"]["id"] = 1;
	response["info"]["name"] = "aarch64 core";
	response["info"]["type"] = "core";
	response["info"]["caps"] = "0x1f";
	return true;
}

bool CVSpaceCmd::getCurInstance(Json::Value &request, Json::Value &response)
{
	response["active"] = 1;
	return true;
}

bool CVSpaceCmd::setActiveInstance(Json::Value &request, Json::Value &response)
{
	if (request.isMember("active") && request["active"].isUInt())
	{
		unsigned int id = request["active"].asUInt();
		//todo
		response["result"] = "success";
	}
	else
	{
		response["result"] = "fail";
		response["errcode"] = ErrFormat;
	}
	return true;
}
bool CVSpaceCmd::getRegsiterGroup(Json::Value &request, Json::Value &response)
{
	response["list"][0]["value"] = 0x123456789;
	response["list"][0]["name"] = "PC";
	response["list"][0]["width"] = 64;
	response["list"][0]["attr"] = "rw";
	return true;
}

bool CVSpaceCmd::getRegsiterList(Json::Value &request, Json::Value &response)
{
	response["list"][0]["value"] = 0x123456789;
	response["list"][0]["name"] = "PC";
	response["list"][0]["width"] = 64;
	response["list"][0]["attr"] = "rw";
	return true;
}

bool CVSpaceCmd::getRegsiter(Json::Value &request, Json::Value &response)
{
	if (response.isMember("name") && response["name"].isString())
	{
		const std::string name = response["name"].asString();
		//todo
		response["result"] = "success";
	}
	else
	{
		response["result"] = "fail";
		response["errcode"] = ErrFormat;
	}
	return true;
}

bool CVSpaceCmd::setRegsiter(Json::Value &request, Json::Value &response)
{
	if (response.isMember("name") && response["name"].isString()
		&& response.isMember("value") && response["value"].isArray())
	{
		//todo
	}
	else
	{
		response["result"] = "fail";
		response["errcode"] = ErrFormat;
	}
	return true;
}

bool CVSpaceCmd::getMemory(Json::Value &request, Json::Value &response)
{
	if (response.isMember("addr") && response["addr"].isUInt64()
		&& response.isMember("len") && response["len"].isUInt64())
	{
		//todo
	}
	else
	{
		response["result"] = "fail";
		response["errcode"] = ErrFormat;
	}
	return true;
}

bool CVSpaceCmd::setMemory(Json::Value &request, Json::Value &response)
{
	if (response.isMember("addr") && response["addr"].isUInt64()
		&& response.isMember("len") && response["len"].isUInt64()
		&& response.isMember("value") && response["value"].isArray())
	{
		//todo
	}
	else
	{
		response["result"] = "fail";
		response["errcode"] = ErrFormat;
	}
	return true;
}


bool CVSpaceCmd::getBreakPoint(Json::Value &request, Json::Value &response)
{

	return true;
}

bool CVSpaceCmd::setBreakPoint(Json::Value &request, Json::Value &response)
{
	if (response.isMember("addr") && response["addr"].isUInt64())
	{
		//todo
	}
	else
	{
		response["result"] = "fail";
		response["errcode"] = ErrFormat;
	}
	return true;
}

bool CVSpaceCmd::delBreakPoint(Json::Value &request, Json::Value &response)
{
	if (response.isMember("addr") && response["addr"].isUInt64())
	{
		//todo
	}
	else
	{
		response["result"] = "fail";
		response["errcode"] = ErrFormat;
	}
	return true;
}

bool CVSpaceCmd::getWatchPoint(Json::Value &request, Json::Value &response)
{
	return true;
}

bool CVSpaceCmd::setWatchPoint(Json::Value &request, Json::Value &response)
{
	if (response.isMember("addr") && response["addr"].isUInt64()
		&& response.isMember("len") && response["len"].isUInt64()
		&& response.isMember("attr") && response["attr"].isString())
	{
		//todo
	}
	else
	{
		response["result"] = "fail";
		response["errcode"] = ErrFormat;
	}
	return true;
}

bool CVSpaceCmd::delWatchPoint(Json::Value &request, Json::Value &response)
{
	if (response.isMember("addr") && response["addr"].isUInt64()
		&& response.isMember("len") && response["len"].isUInt64()
		&& response.isMember("attr") && response["attr"].isString())
	{
		//todo
	}
	else
	{
		response["result"] = "fail";
		response["errcode"] = ErrFormat;
	}
	return true;
}

bool CVSpaceCmd::cmdRun(Json::Value &request, Json::Value &response)
{
	response["result"] = "success";
	return true;
}

bool CVSpaceCmd::cmdStop(Json::Value &request, Json::Value &response)
{
	response["result"] = "success";
	return true;
}

bool CVSpaceCmd::cmdStep(Json::Value &request, Json::Value &response)
{
	response["result"] = "success";
	return true;
}


void CVSpaceCmd::sendPacket(Json::Value & packet)
{
	std::ostringstream os;
	Json::StreamWriterBuilder writerBuilder;
	writerBuilder["indentation"] = "";
	writerBuilder["emitUTF8"] = true;
	std::unique_ptr<Json::StreamWriter> const jsonWriter(writerBuilder.newStreamWriter());
	jsonWriter->write(packet, &os);
	std::string reString = os.str();
	reString += " ";
	unsigned short csum = checkSum((const unsigned char*)reString.c_str(), reString.length());

	const int length = sizeof(struct stVcpHeader) + reString.length();
	char* p = new char [length];
	struct stVcpHeader * pVcpHdr = (struct stVcpHeader *)p;
	pVcpHdr->ident = htonl(0x7E766370);
	pVcpHdr->reserve = 0;
	pVcpHdr->fsc = htons(csum);
	pVcpHdr->length = htonl(length);
	memcpy(p + sizeof(struct stVcpHeader), reString.c_str(), reString.length());

	send(p, length);
	delete p;
}

unsigned short CVSpaceCmd::checkSum(const unsigned char* buffer, int size)
{
	unsigned long cksum = 0;
	unsigned short * pbuf = (unsigned short *)buffer;
	for (int i = 0; i < size / 2; i++)
	{
		cksum += htons(pbuf[i]);
	}
	
	if( size % 2 == 1) 
	{ 
		cksum += (unsigned short)(buffer[size - 1] << 8);
	}
	
	while (cksum >> 16) 
	{
		cksum = (cksum >> 16) + (cksum & 0xffff); 
	}
	
	return (unsigned short)(~cksum);
}
