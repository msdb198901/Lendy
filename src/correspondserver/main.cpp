#include "Define.h"
#include "IOContext.h"
#include "Log.h"
#include "ServiceUnits.h"

int main(int argc, char** argv)
{
	using namespace LogComm;

	sLogMgr->Initialize("CorrespondConfig.ini");

	LOG_INFO("server.correspond", "CorrespondServer is initializing...");
	
	std::shared_ptr<Net::IOContext> ioContext = std::make_shared<Net::IOContext>();

	SrvUnitsMgr->Start(ioContext.get());

	ioContext->run();

	return 0;
}