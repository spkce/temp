
#include "terminal.h"
#include "vSpaceCmd.h"
#include "cmdServer.h"

class CVcpTerminal : public NetFwk::ITerminal
{
public:
	CVcpTerminal();
	virtual ~CVcpTerminal();
	virtual bool init(unsigned int port, int timeout);
	virtual NetFwk::IProtocol* createProtocol(NetFwk::ISession* session, size_t recvlen);
};

CVcpTerminal::CVcpTerminal()
:NetFwk::ITerminal(5, 153)
{
}

CVcpTerminal::~CVcpTerminal()
{
}

bool CVcpTerminal::init(unsigned int port, int timeout)
{
	return ITerminal::init(port, timeout);
}

NetFwk::IProtocol* CVcpTerminal::createProtocol(NetFwk::ISession* session, size_t recvlen)
{
	return new CVSpaceCmd(session, recvlen);
}

CCmdServer::CCmdServer()
{
	m_terminal = new CVcpTerminal();
}

CCmdServer::~CCmdServer()
{
	delete m_terminal;
}

CCmdServer* CCmdServer::instance()
{
	static CCmdServer instance;
	return &instance;
}

bool CCmdServer::init(unsigned port)
{
	return m_terminal->init(port, 15);
}

bool CCmdServer::stop()
{
	return m_terminal->stop();
}