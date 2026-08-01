@echo off
REM Build battle simulator (console, hien dice rolls)
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
set OUTDIR=build
if not exist %OUTDIR% mkdir %OUTDIR%

echo === Build Battle Simulator ===
cl /nologo /MT /O2 /utf-8 /Fe:%OUTDIR%\battle_sim.exe ^
   /I src_console ^
   /DSCREEN_W=100 /DSCREEN_H=50 ^
   src_console\engine\rng.c ^
   src_console\engine\map.c ^
   src_console\engine\fov.c ^
   src_console\engine\path.c ^
   src_console\engine\bsp.c ^
   src_console\game\actor.c ^
   src_console\game\d20.c ^
   src_console\game\combat.c ^
   src_console\game\conditions.c ^
   src_console\game\inventory.c ^
   src_console\game\ai.c ^
   src_console\game\spell_resolve.c ^
   src_console\game\save.c ^
   src_console\data\monsters.c ^
   src_console\data\items.c ^
   src_console\data\spells.c ^
   src_console\game\battle_sim.c ^
   /link /SUBSYSTEM:CONSOLE ^
   user32.lib kernel32.lib legacy_stdio_definitions.lib

if errorlevel 1 (echo [LOI] Build that bai & exit /b 1)
echo.
echo === DONE: %OUTDIR%\battle_sim.exe ===
