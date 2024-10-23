@echo off
rem файл для программирования целевого ПО
CLS

echo Program project run...

"C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\ST-Link_CLI.exe" -c SWD FREQ=4000 -P "build\RF_HACK.hex" -V -Rst

TIMEOUT /T 10
