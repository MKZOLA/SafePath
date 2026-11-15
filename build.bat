@echo off
gcc main.c -o safepath.exe -lraylib -lopengl32 -lgdi32 -lwinmm
if errorlevel 1 (
    echo Build failed. Check that GCC and raylib are installed and on PATH.
    pause
    exit /b 1
)
echo Build successful. Run safepath.exe to start SafePath.
pause
