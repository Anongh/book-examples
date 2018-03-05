@echo off

set dsar_sartol=sartol.exe
set dsar_bpath="%USERPROFILE%/데스크탑/"
set dsar_autorun="%SystemRoot%\explorer.exe"

rem －예-
rem set dsar_bpath="%USERPROFILE%/데스크탑/"
rem set dsar_bpath="%USERPROFILE%/데스크탑"
rem set dsar_bpath=..@arcpath/
rem set dsar_bpath=..@arcpath

rem －예-
rem set dsar_autorun="%SystemRoot%\explorer.exe"
rem set dsar_autorun=
rem 주의! dsar_bpath에서는 패스의 단락에 \를 사용할 것

:loop
if %1. ==.  goto end
%dsar_sartol% d %1 %dsar_bpath% %dsar_autorun%
shift
goto loop
:end