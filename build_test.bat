@echo off
REM Build logic test harness (CONSOLE subsystem - in printf ket qua)
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
set OUTDIR=build
if not exist %OUTDIR% mkdir %OUTDIR%

echo === Build Logic Test Harness ===
cl /nologo /MT /O2 /utf-8 /Fe:%OUTDIR%\test_logic.exe ^
   /I src_console ^
   /DSCREEN_W=100 /DSCREEN_H=50 ^
   src_console\engine\rng.c ^
   src_console\engine\map.c ^
   src_console\engine\fov.c ^
   src_console\engine\path.c ^
   src_console\engine\bsp.c ^
   src_console\engine\inflate.c ^
   src_console\engine\xp_loader.c ^
   src_console\game\actor.c ^
   src_console\game\d20.c ^
   src_console\game\combat.c ^
   src_console\game\conditions.c ^
   src_console\game\inventory.c ^
   src_console\game\ai.c ^
   src_console\game\spell_resolve.c ^
   src_console\game\distance.c ^
   src_console\game\save.c ^
   src_console\data\monsters.c ^
   src_console\data\items.c ^
   src_console\data\spells.c ^
   src_console\game\test_logic.c ^
   /link /SUBSYSTEM:CONSOLE ^
   user32.lib kernel32.lib legacy_stdio_definitions.lib

if errorlevel 1 (echo [LOI] Build that bai & exit /b 1)
echo.
echo === DONE: %OUTDIR%\test_logic.exe ===
