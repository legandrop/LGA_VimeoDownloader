@echo off
setlocal
rem Sincroniza los tres espejos de reglas (AGENTS.md / CLAUDE.md / .cursor).
rem La logica esta en tools\sync_rules.py; esto es solo el wrapper de Windows.
set "SCRIPT=%~dp0tools\sync_rules.py"
if not exist "%SCRIPT%" (
    echo ERROR: No se encontro "%SCRIPT%".
    exit /b 1
)

set "EMBEDDED_PY=%~dp0python_runtime\windows\python.exe"
if exist "%EMBEDDED_PY%" (
    "%EMBEDDED_PY%" "%SCRIPT%" %*
) else (
    py -3 "%SCRIPT%" %* 2>nul || python "%SCRIPT%" %*
)
endlocal
