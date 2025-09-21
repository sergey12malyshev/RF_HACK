cls
@echo off
rem automatic assembly of the release archive with the archive name set according to the latest GIT tag
chcp 1251

echo start generate release...

PowerShell.exe -noexit -executionpolicy bypass -executionpolicy bypass -File "scripts/generateRelease.ps1"

exit