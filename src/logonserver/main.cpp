#include "Define.h"
#include "IOContext.h"
#include "Log.h"
#include "ServiceUnits.h"

int main(int argc, char** argv)
{
	using namespace LogComm;

	sLogMgr->Initialize("LogonConfig.ini");

	LOG_INFO("server.logon", "µÇÂ¼·þÎñÆ÷Æô¶¯");
	
	std::shared_ptr<Net::IOContext> ioContext = std::make_shared<Net::IOContext>();

	SrvUnitsMgr->Start(ioContext.get());

	asio::io_context::work work(*ioContext);
	ioContext->run();

	return 0;
}