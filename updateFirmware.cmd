@echo off
rem Upload the program file to the 0x08000000 flash memory address use usb-uart adapter
CLS

echo Upload project run...

D:\CubeProgrammer\bin\STM32_Programmer_CLI.exe -c port=/COM4 br=115200 -w build\RF_HACK.hex --go


pause