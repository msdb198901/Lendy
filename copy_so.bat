@echo off

set config_file_path="F:\Learn\lendy\src\logonserver"
set src_file_path="F:\Learn\lendy\Build\apilib"
set dst_file_path="F:\Learn\lendy\Build\src\logonserver"


xcopy %src_file_path%\dep\fmt\Debug\*.lib %dst_file_path% /s /e /y
xcopy %src_file_path%\util\Debug\*.lib %dst_file_path%/s /e /y
xcopy %src_file_path%\util\Debug\*.dll %dst_file_path% /s /e /y
xcopy %src_file_path%\log\Debug\*.lib %dst_file_path% /s /e /y
xcopy %src_file_path%\log\Debug\*.dll %dst_file_path% /s /e /y
xcopy %src_file_path%\Net\Debug\*.lib %dst_file_path% /s /e /y
xcopy %src_file_path%\Net\Debug\*.dll %dst_file_path% /s /e /y
xcopy %src_file_path%\db\Debug\*.lib %dst_file_path% /s /e /y
xcopy %src_file_path%\db\Debug\*.dll %dst_file_path% /s /e /y

xcopy %src_file_path%\Debug\*.lib %dst_file_path% /s /e /y
xcopy %src_file_path%\Debug\*.dll %dst_file_path% /s /e /y
xcopy %config_file_path%\*.ini %dst_file_path% /s /e /y


xcopy %src_file_path%\dep\fmt\Debug\*.lib %dst_file_path%\Debug /s /e /y
xcopy %src_file_path%\util\Debug\*.lib %dst_file_path%\Debug  /s /e /y
xcopy %src_file_path%\util\Debug\*.dll %dst_file_path%\Debug /s /e /y
xcopy %src_file_path%\log\Debug\*.lib %dst_file_path%\Debug /s /e /y
xcopy %src_file_path%\log\Debug\*.dll %dst_file_path%\Debug /s /e /y
xcopy %src_file_path%\Net\Debug\*.lib %dst_file_path%\Debug /s /e /y
xcopy %src_file_path%\Net\Debug\*.dll %dst_file_path%\Debug /s /e /y
xcopy %src_file_path%\db\Debug\*.lib %dst_file_path%\Debug /s /e /y
xcopy %src_file_path%\db\Debug\*.dll %dst_file_path%\Debug /s /e /y

xcopy %src_file_path%\Debug\*.lib %dst_file_path%\Debug /s /e /y
xcopy %src_file_path%\Debug\*.dll %dst_file_path%\Debug /s /e /y
xcopy %config_file_path%\*.ini %dst_file_path%\Debug /s /e /y

pause