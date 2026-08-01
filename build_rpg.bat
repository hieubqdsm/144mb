@echo off
REM Build full RPG (engine + game + data modules)
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
set OUTDIR=build
if not exist %OUTDIR% mkdir %OUTDIR%

echo === Build ASCII Dungeon Crawler RPG ===
cl /nologo /MT /O2 /utf-8 /Fe:%OUTDIR%\rpg.exe ^
   /I src_console ^
   /DSCREEN_W=100 /DSCREEN_H=50 ^
   src_console\engine\console.c ^
   src_console\engine\rng.c ^
   src_console\engine\map.c ^
   src_console\engine\fov.c ^
   src_console\engine\path.c ^
   src_console\engine\bsp.c ^
   src_console\game\actor.c ^
   src_console\game\d20.c ^
   src_console\game\combat.c ^
   src_console\game\turn.c ^
   src_console\game\conditions.c ^
   src_console\game\inventory.c ^
   src_console\game\ai.c ^
   src_console\game\spell_resolve.c ^
   src_console\game\dungeon.c ^
   src_console\game\ui.c ^
   src_console\data\monsters.c ^
   src_console\data\items.c ^
   src_console\data\spells.c ^
   src_console\game\rpg_main.c ^
   /link /SUBSYSTEM:CONSOLE ^
   user32.lib kernel32.lib winmm.lib legacy_stdio_definitions.lib

if errorlevel 1 (echo [LOI] Build that bai & exit /b 1)
echo.
echo === DONE: %OUTDIR%\rpg.exe ===
for %%A in (%OUTDIR%\rpg.exe) do echo   exe: %%~zA bytes
