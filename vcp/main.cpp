
#include "stdio.h"
#include "Log.h"
#include "cmdServer.h"
#include "jsoncpp.h"
#include "ByteBuffer.h"
#include <list>
#include <arpa/inet.h>
namespace ii
{
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

				bool ret = parser(buf, length, csum);
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


}


short checkSum(const unsigned char* buffer, int size)
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

int main(int argc, char const *argv[])
{
	Infra::CLog* pLog = Infra::CLogManager::instance()->getLog("netFwk");
	pLog->setLevel(Infra::CLog::logLevel_5);
	pLog = Infra::CLogManager::instance()->getLog("CmdServer");
	pLog->setLevel(Infra::CLog::logLevel_5);

	CCmdServer::instance()->init(8000);

	while (1);
	#if 0
struct stVcpHeader
{
	unsigned int ident;
	unsigned short reserve;
	unsigned short fsc;
	unsigned int length;
};
	char buf[1024] = {0};


	unsigned char* p = (unsigned char*)buf;
	struct stVcpHeader * phdr = (struct stVcpHeader *)p;

	const char* str1 = "{\"id\":1,\"group\":\"AArch32 VFP/NEON\", \"name\":\"Q1\",\"msgID\":107, \"sessionID\":140214441877680}";
	const int len1 = strlen(str1);
	unsigned char* pBody = p + sizeof(struct stVcpHeader);

	phdr->ident = htonl(0x7E766370);
	phdr->length = htonl(len1 + sizeof(struct stVcpHeader));
	phdr->fsc = htons(checkSum((unsigned char*)str1, len1));
	memcpy(pBody, str1, len1);

	p += len1 + sizeof(struct stVcpHeader);

	const char* str2 = "{\"id\":1,\"group\":\"AArch32 VFP/NEON\", \"name\":\"Q2\",\"msgID\":107, \"sessionID\":140214441877680}";
	const int len2 = strlen(str2);
	phdr = (struct stVcpHeader *)p;
	pBody = p + sizeof(struct stVcpHeader);
	
	phdr->ident = htonl(0x7E766370);
	phdr->length = htonl(len2 + sizeof(struct stVcpHeader));
	phdr->fsc = htons(checkSum((unsigned char*)str2, len2));
	memcpy(pBody, str2, len2);

	//client.send((char*)p, len2 + sizeof(struct stVcpHeader));

	p += len2 + sizeof(struct stVcpHeader);

	const char* str3 = "{\"id\":1,\"group\":\"AArch32 VFP/NEON\", \"name\":\"Q3\",\"msgID\":107, \"sessionID\":140214441877680}";
	const int len3 = strlen(str3);
	phdr = (struct stVcpHeader *)p;
	pBody = p + sizeof(struct stVcpHeader);

	phdr->ident = htonl(0x7E766370);
	phdr->length = htonl(len3 + sizeof(struct stVcpHeader));
	phdr->fsc = htons(checkSum((unsigned char*)str3, len3));
	memcpy(pBody, str3, len3);

	ii::CVcpParser m_parser;


	m_parser.input((unsigned char*)buf, 102+2);
	m_parser.input((unsigned char*)buf + 102+2, 202);
	printf("\033[0;35m""*************************************\n""\033[0m\n");
	Json::Value request = Json::nullValue;
	int len = m_parser.output(request);
	printf("\033[0;35m""len = %d\n""\033[0m\n", len);
	printf("\033[0;35m""request = %s\n""\033[0m\n", request.toStyledString().c_str());

	while (m_parser.output(request) >= 0)
	{
		printf("\033[0;35m""request = %s\n""\033[0m\n", request.toStyledString().c_str());
	}
	#endif 
	return 0;
}
