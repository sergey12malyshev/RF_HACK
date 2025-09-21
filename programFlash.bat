@echo off
rem a file for programming target software
CLS

echo Program project run...

"C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-Link_CLI.exe" -c SWD FREQ=4000 -P "build\RF_HACK.hex" -V -Rst

TIMEOUT /T 10
exit