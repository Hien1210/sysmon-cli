@echo off
setlocal

echo ===== sysmon-cli Release Build v1.0 =====

where g++ >nul 2>nul
if errorlevel 1 set "PATH=C:\mingw64\bin;%PATH%"

where g++ >nul 2>nul
if errorlevel 1 (
    echo [ERROR] Khong tim thay g++. Hay cai MinGW-w64 va them vao PATH.
    exit /b 1
)

set "OUT_DIR=bin\release"
set "OUT_EXE=%OUT_DIR%\sysmon-cli.exe"

if exist "%OUT_DIR%" (
    echo Don dep thu muc cu: %OUT_DIR%
    rmdir /s /q "%OUT_DIR%"
)
mkdir "%OUT_DIR%"

echo Dang bien dich release build (static, toi uu -O2, loai bo debug symbol)...
g++ -std=c++17 -O2 -s -static -static-libgcc -static-libstdc++ -Iinclude src\main.cpp src\system_collector.cpp src\terminal_ui.cpp src\process_manager.cpp -o "%OUT_EXE%" -lpsapi

if errorlevel 1 (
    echo [ERROR] Bien dich that bai.
    exit /b 1
)

echo [OK] Build thanh cong: %OUT_EXE%
for %%F in ("%OUT_EXE%") do echo Dung luong: %%~zF bytes

endlocal
