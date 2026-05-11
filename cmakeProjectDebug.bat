@echo off
echo ========================================
echo Building DEBUG version (with debug symbols)
echo ========================================
echo Cleaning old build...
rmdir /s /q build

cmake -B build -G "MinGW Makefiles" ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DDEBUG_MAIN=ON ^
    -DUSE_FULL_ASSERT=ON ^
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
cmake --build build -j2

if %errorlevel% neq 0 (
    echo.
    echo Build failed!
    pause
    exit /b %errorlevel%
)

echo.
echo ========================================
echo DEBUG build complete!
echo Optimization: -Og (same as release)
echo Debug symbols: YES (-g -gdwarf-2)
echo Additional defines: DEBUG_MAIN, USE_FULL_ASSERT
echo ========================================
pause