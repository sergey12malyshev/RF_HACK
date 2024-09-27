@echo off
rem срипт запускает статический анализ кода проекта используюя утилиту Cppcheck
rem для работы необходимо установить https://cppcheck.sourceforge.io/ и прописать путь в PATH
rem Гайд: https://habr.com/ru/articles/210256/
@echo on

rem вывод результата в консоль
cppcheck -q -j4 --enable=all --inconclusive -I ./Core/Inc ./Core/Src
rem вывод результата в файл
rem cppcheck -q -j4 --enable=all --inconclusive --output-file=checkReport.txt  ./Src

PAUSE