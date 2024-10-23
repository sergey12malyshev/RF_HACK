@echo off
rem соборка проекта
CLS

echo Clean project...
make clean

echo Build project...
make -j1

pause
