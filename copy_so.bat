@echo off

set mode="Release"
set mode="Debug"

set src_file_path="%~dp0Build\apilib"

set config_src="%~dp0src\correspondserver" "%~dp0src\logonserver" "%~dp0src\gameserver"
set dst_run="%~dp0Build\src\correspondserver" "%~dp0Build\src\logonserver" "%~dp0Build\src\gameserver" "%~dp0Build\src\correspondserver\%mode%" "%~dp0Build\src\logonserver\%mode%" "%~dp0Build\src\gameserver\%mode%"

(FOR %%a in (%dst_run%) DO ( 
	xcopy %src_file_path%\dep\fmt\%mode%\*.lib 	%%a /s /e /y
	xcopy %src_file_path%\util\%mode%\*.lib 	%%a /s /e /y
	xcopy %src_file_path%\util\%mode%\*.dll 	%%a /s /e /y
	xcopy %src_file_path%\log\%mode%\*.lib 		%%a /s /e /y
	xcopy %src_file_path%\log\%mode%\*.dll 		%%a /s /e /y
	xcopy %src_file_path%\Net\%mode%\*.lib 		%%a /s /e /y
	xcopy %src_file_path%\Net\%mode%\*.dll 		%%a /s /e /y
	xcopy %src_file_path%\db\%mode%\*.lib 		%%a /s /e /y
	xcopy %src_file_path%\db\%mode%\*.dll 		%%a /s /e /y
	
	(FOR %%b in (%config_src%) DO ( 
		xcopy %%b\*.ini 						%%a /s /e /y
	))
))




