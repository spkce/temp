
#include "stdio.h"
#include "Log.h"
#include "cmdServer.h"

int main(int argc, char const *argv[])
{
	Infra::CLog* pLog = Infra::CLogManager::instance()->getLog("netFwk");
	pLog->setLevel(Infra::CLog::logLevel_5);
	pLog = Infra::CLogManager::instance()->getLog("CmdServer");
	pLog->setLevel(Infra::CLog::logLevel_5);

	CCmdServer::instance()->init(8000);

	while (1);
	
	return 0;
}
