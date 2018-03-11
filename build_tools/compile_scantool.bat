SET path=%PATH%;C:\MINGW\BIN
SET MINGDIR=C:\MINGW

REM Replace the location in the next line with your source code directory location
CD c:\scantool

MD compiled
mingw32-make NOWERROR=1 -lalleg -o compiled\SCANTOOL.exe
COPY scantool.dat compiled\scantool.dat
COPY %MINGDIR%\bin\alleg42.dll compiled\alleg42.dll
pause