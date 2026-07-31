@echo off
REM Build PEWBALL (KitConsole engine) bang MSVC
setlocal
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul

set OUTDIR=build
if not exist %OUTDIR% mkdir %OUTDIR%

echo === Compile PEWBALL (C++, Console subsystem) ===
REM /EHsc = C++ exception handling
REM /utf-8 = comment trong source
REM Include ca root (game.h, sound.h) va PewBall/ (cac state header)
cl /nologo /MT /O2 /EHsc /utf-8 /DWIN32 /D_WINDOWS ^
   /I src /I src\PewBall ^
   src\sound.cpp src\PewBall\pewball.cpp ^
   /Fe:%OUTDIR%\pewball.exe ^
   /link /SUBSYSTEM:CONSOLE ^
   user32.lib kernel32.lib winmm.lib

if errorlevel 1 (
    echo [LOI] Build that bai
    exit /b 1
)

echo.
echo === Copy audio assets ===
xcopy /Y /I /E src\PewBall\Audio %OUTDIR%\Audio >nul
echo Audio copied.

echo.
echo === DONE: %OUTDIR%\pewball.exe ===
for %%A in (%OUTDIR%\pewball.exe) do echo   exe: %%~zA bytes
endlocal
