@echo off
echo ========================================
echo Building RELEASE version (no debug symbols)
echo ========================================
echo Cleaning old build...
rmdir /s /q build

cmake -B build -G "MinGW Makefiles" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake ^
    -DCMAKE_MAKE_PROGRAM="C:/Program Files (x86)/xpack-windows-build-tools-4.4.0-1-win32-x64/xpack-windows-build-tools-4.4.0-1/bin/make.exe"

if %errorlevel% neq 0 (
    echo.
    echo CMake configuration failed!
    pause
    exit /b %errorlevel%
)

echo.
echo Building project...
cmake --build build -j1

if %errorlevel% neq 0 (
    echo.
    echo Build failed!
    pause
    exit /b %errorlevel%
)

echo.
echo ========================================
echo RELEASE build complete!
echo Optimization: -Og (like original Makefile)
echo Debug symbols: NO
echo Defines: USE_FULL_LL_DRIVER, STM32F401xC, USE_HAL_DRIVER
echo ========================================
pause