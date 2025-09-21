@echo off
rem build debug project
CLS

echo Clean project...
make clean

echo Build project Debug...
make -j2 debug

pause
