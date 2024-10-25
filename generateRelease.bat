cls
@echo off
rem автоматическая сборка релизного архива с установкой имени архива согласно последнему GIT-тегу
chcp 1251

echo generateRelease...

PowerShell.exe -noexit -executionpolicy bypass -executionpolicy bypass -File "scripts/generateRelease.ps1"

exit