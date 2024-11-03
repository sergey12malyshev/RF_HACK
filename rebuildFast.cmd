@echo off
rem Fast rebuild
cls
echo rebuild run...

make clean

make -j

pause
exit