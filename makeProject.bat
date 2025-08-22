@echo off
rem build project
CLS

echo Clean project...
make clean

echo Build project...
make -j1

pause
