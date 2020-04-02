#include "Define.h"
#include "IOContext.h"
#include "Log.h"
#include "ServiceUnits.h"

int main(int argc, char** argv)
{
	using namespace LogComm;

	sLogMgr->Initialize("LogonConfig.ini");

	if (argc < 11)
	{
		LOG_INFO("server.game", "you should start by \"%s  -h 192.168.1.217 -p 7000 -s 1 -t 200 -m 1 ...\"\n", argv[0]);
		LOG_INFO("server.game", "============================= 帮助 ==============================================");
		LOG_INFO("server.game", "");
		LOG_INFO("server.game", "==================== -h IP  指定要监听的IP地址       =============================");
		LOG_INFO("server.game", "==================== -p PORT 指定要监听的端口        =============================");
		LOG_INFO("server.game", "==================== -d 没有参数，指定后台运行       =============================");
		LOG_INFO("server.game", "==================== -s ServerID 指定游戏服务器ID    =============================");
		LOG_INFO("server.game", "==================== -t KindID 指定游戏类型          =============================");
		LOG_INFO("server.game", "==================== -m GameMod  指定游戏模式        =============================");
		LOG_INFO("server.game", "");
		LOG_INFO("server.game", "===================================================================================");
		return -1;
	}

	LOG_INFO("server.logon", "登录服务器启动");
	
	std::shared_ptr<Net::IOContext> ioContext = std::make_shared<Net::IOContext>();

	SrvUnitsMgr->Start(ioContext.get());

	asio::io_context::work work(*ioContext);
	ioContext->run();

	return 0;
}