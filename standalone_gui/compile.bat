@echo off
:: Windows compilation script for FolderCreator

echo 正在编译 FolderCreator...

:: Check if g++ is available
g++ --version >nul 2>&1
if %errorlevel% neq 0 (
    echo 错误: 未找到 g++ 编译器。请安装 MinGW-w64 或 TDM-GCC。
    pause
    exit /b 1
)

:: Check if FLTK is available
fltk-config --version >nul 2>&1
if %errorlevel% neq 0 (
    echo 警告: 未找到 FLTK 开发库。请安装 FLTK 并确保 fltk-config 在 PATH 中。
    echo 尝试使用默认设置编译...
)

:: Compile with static linking to minimize dependencies
g++ -static -std=c++17 -O2 main.cpp -o FolderCreator.exe -lfltk -lcomctl32 -lole32 -luuid -lcomdlg32

if %errorlevel% equ 0 (
    echo 编译成功完成!
    echo 可执行文件: FolderCreator.exe
) else (
    echo 编译失败!
    echo 请确保已正确安装 FLTK 库和开发文件。
)

pause