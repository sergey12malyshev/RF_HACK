@echo off
rem Upload the program file to the 0x08000000 flash memory address use usb-uart adapter
CLS

SET comport=COM6

echo Use port: %comport%
echo Upload project run...

D:\CubeProgrammer\bin\STM32_Programmer_CLI.exe -c port=%comport% br=115200 -w build\RF_HACK.hex --go

pause
exit