@echo off
rem Script launches a static analysis of the project code using the Cppcheck utility
rem To work, you need to install https://cppcheck.sourceforge.io / and specify the path in the PATH
rem The Guide: https://habr.com/ru/articles/210256/
echo Run static analysis...
@echo on

@rem output of the result to the console:
cppcheck -q -j4 --enable=all --inconclusive -I ./Core/Inc ./Core/Src
@rem output of the result to a file:
rem cppcheck -q -j4 --enable=all --inconclusive --output-file=checkReport.txt  ./Src

pause
exit