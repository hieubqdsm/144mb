@echo off
REM Build D&D demo (engine moi tach module)
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
set OUTDIR=build
if not exist %OUTDIR% mkdir %OUTDIR%

echo === Build D&D ASCII demo (modular engine) ===
cl /nologo /MT /O2 /utf-8 /Fe:%OUTDIR%\dd.exe ^
   /I src_console ^
   src_console\engine\console.c ^
   src_console\dd_demo.c ^
   /link /SUBSYSTEM:CONSOLE ^
   user32.lib kernel32.lib winmm.lib legacy_stdio_definitions.lib

if errorlevel 1 (echo [LOI] Build that bai & exit /b 1)
echo.
echo === DONE: %OUTDIR%\dd.exe ===
for %%A in (%OUTDIR%\dd.exe) do echo   exe: %%~zA bytes
