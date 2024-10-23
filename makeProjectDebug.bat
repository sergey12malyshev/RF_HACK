@echo off
rem cборка debug проекта
CLS

echo Clean project...
make clean

echo Build project Debug...
make -j2 debug

pause
