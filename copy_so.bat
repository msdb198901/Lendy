@echo off

set config_file_path="%~dp0src\logonserver"
set game_config_file_path="%~dp0src\gameserver"
set src_file_path="%~dp0Build\apilib"
set dst_file_path="%~dp0Build\src\logonserver"
set dst_game_path="%~dp0Build\src\gameserver"

set mode="Release"
set mode="Debug"
xcopy %src_file_path%\dep\fmt\%mode%\*.lib %dst_file_path% /s /e /y
xcopy %src_file_path%\util\%mode%\*.lib %dst_file_path%/s /e /y
xcopy %src_file_path%\util\%mode%\*.dll %dst_file_path% /s /e /y
xcopy %src_file_path%\log\%mode%\*.lib %dst_file_path% /s /e /y
xcopy %src_file_path%\log\%mode%\*.dll %dst_file_path% /s /e /y
xcopy %src_file_path%\Net\%mode%\*.lib %dst_file_path% /s /e /y
xcopy %src_file_path%\Net\%mode%\*.dll %dst_file_path% /s /e /y
xcopy %src_file_path%\db\%mode%\*.lib %dst_file_path% /s /e /y
xcopy %src_file_path%\db\%mode%\*.dll %dst_file_path% /s /e /y

xcopy %src_file_path%\%mode%\*.lib %dst_file_path% /s /e /y
xcopy %src_file_path%\%mode%\*.dll %dst_file_path% /s /e /y
xcopy %config_file_path%\*.ini %dst_file_path% /s /e /y


xcopy %src_file_path%\dep\fmt\%mode%\*.lib %dst_file_path%\%mode% /s /e /y
xcopy %src_file_path%\util\%mode%\*.lib %dst_file_path%\%mode%  /s /e /y
xcopy %src_file_path%\util\%mode%\*.dll %dst_file_path%\%mode% /s /e /y
xcopy %src_file_path%\log\%mode%\*.lib %dst_file_path%\%mode% /s /e /y
xcopy %src_file_path%\log\%mode%\*.dll %dst_file_path%\%mode% /s /e /y
xcopy %src_file_path%\Net\%mode%\*.lib %dst_file_path%\%mode% /s /e /y
xcopy %src_file_path%\Net\%mode%\*.dll %dst_file_path%\%mode% /s /e /y
xcopy %src_file_path%\db\%mode%\*.lib %dst_file_path%\%mode% /s /e /y
xcopy %src_file_path%\db\%mode%\*.dll %dst_file_path%\%mode% /s /e /y

xcopy %src_file_path%\%mode%\*.lib %dst_file_path%\%mode% /s /e /y
xcopy %src_file_path%\%mode%\*.dll %dst_file_path%\%mode% /s /e /y
xcopy %config_file_path%\*.ini %dst_file_path%\%mode% /s /e /y


@////////////////////////////////////////////////
xcopy %src_file_path%\dep\fmt\%mode%\*.lib %dst_game_path% /s /e /y
xcopy %src_file_path%\util\%mode%\*.lib %dst_game_path%/s /e /y
xcopy %src_file_path%\util\%mode%\*.dll %dst_game_path% /s /e /y
xcopy %src_file_path%\log\%mode%\*.lib %dst_game_path% /s /e /y
xcopy %src_file_path%\log\%mode%\*.dll %dst_game_path% /s /e /y
xcopy %src_file_path%\Net\%mode%\*.lib %dst_game_path% /s /e /y
xcopy %src_file_path%\Net\%mode%\*.dll %dst_game_path% /s /e /y
xcopy %src_file_path%\db\%mode%\*.lib %dst_game_path% /s /e /y
xcopy %src_file_path%\db\%mode%\*.dll %dst_game_path% /s /e /y

xcopy %src_file_path%\%mode%\*.lib %dst_game_path% /s /e /y
xcopy %src_file_path%\%mode%\*.dll %dst_game_path% /s /e /y
xcopy %config_file_path%\*.ini %dst_game_path% /s /e /y
xcopy %game_config_file_path%\*.ini %dst_game_path% /s /e /y

