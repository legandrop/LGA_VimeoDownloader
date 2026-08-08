@echo off
setlocal
python "%~dp0tools\sync_version.py" %*
endlocal
