@echo off
REM ============================================================
REM  build_console.bat - Build CONSOLE ENGINE (modular)
REM  Engine: src_console\engine\*.c
REM  Game:   src_console\main.c
REM ============================================================
setlocal
set VCVARS="C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
set OUTDIR=build
set EXE=%OUTDIR%\game.exe

if not exist %VCVARS% (
    echo [LOI] Khong tim thay vcvars64.bat
    exit /b 1
)
call %VCVARS% >nul
if errorlevel 1 ( echo [LOI] vcvars64 that bai & exit /b 1 )

if not exist %OUTDIR% mkdir %OUTDIR%

echo === Compile + Link (CONSOLE ENGINE modular, /MT /O2) ===
cl /nologo /MT /O2 /utf-8 /Fe:%EXE% ^
   /I src_console ^
   src_console\engine\console.c ^
   src_console\main.c ^
   /link /SUBSYSTEM:CONSOLE ^
   user32.lib kernel32.lib winmm.lib legacy_stdio_definitions.lib

if errorlevel 1 (
    echo [LOI] Build that bai
    exit /b 1
)

echo.
echo === DONE: %EXE% ===
for %%A in (%EXE%) do echo   %%~zA bytes
endlocal
