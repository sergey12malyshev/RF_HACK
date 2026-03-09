@echo off
rem Script for initial programming using JLink

echo =============================================
echo   STM32F401CC JLink programming 
echo =============================================

set jlinkcmd="C:\Program Files\SEGGER\JLink\JLink"
set jlinkopt=-device STM32F401CC -if SWD -speed 3000 -Autoconnect 1 -ExitOnError 1
set hexfile="build\RF_HACK.hex"
set jlinkfile="%TEMP%\STM32F401CC.jlink"

echo connect >%jlinkfile%
echo r0 >>%jlinkfile%
echo r >>%jlinkfile%
echo loadfile %hexfile% >>%jlinkfile%
echo r0 >>%jlinkfile%
echo r >>%jlinkfile%
echo g >>%jlinkfile%
echo q >>%jlinkfile%


%jlinkcmd% %jlinkopt% -CommandFile %jlinkfile%

del %jlinkfile%

pause
exti