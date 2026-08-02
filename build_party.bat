@echo off
REM Build party demo (console DM narrator + 4 hero + initiative + combat)
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
set OUTDIR=build
if not exist %OUTDIR% mkdir %OUTDIR%

echo === Build Party Demo (console) ===
cl /nologo /MT /O2 /utf-8 /Fe:%OUTDIR%\party_demo.exe ^
   /I src_console ^
   src_console\engine\rng.c ^
   src_console\game\actor.c ^
   src_console\game\d20.c ^
   src_console\game\combat.c ^
   src_console\game\turn.c ^
   src_console\game\conditions.c ^
   src_console\game\inventory.c ^
   src_console\game\ai.c ^
   src_console\data\monsters.c ^
   src_console\data\items.c ^
   src_console\game\party_demo.c ^
   /link /SUBSYSTEM:CONSOLE

if errorlevel 1 (echo [LOI] Build that bai & exit /b 1)
echo.
echo === DONE: %OUTDIR%\party_demo.exe ===
for %%A in (%OUTDIR%\party_demo.exe) do echo   exe: %%~zA bytes
