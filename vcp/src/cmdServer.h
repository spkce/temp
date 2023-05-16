#ifndef __CMD_SERVER_H___
#define __CMD_SERVER_H___

class CVcpTerminal;
class CCmdServer
{
protected:
	CCmdServer();
	virtual ~CCmdServer();
public:
	static CCmdServer* instance();

	virtual bool init(unsigned port);
	virtual bool stop();

private:
	CVcpTerminal* m_terminal;
};



#endif //__CMD_SERVER_H___
