### RF_HACK

Платформа для тестирования беспроводной передачи и радиоинтерфейсов 

### MCU
STM32F401CC, High-performance access line, Arm Cortex-M4 core with DSP and FPU, 256 Kbytes of Flash memory, 84 MHz CPU, ART Accelerator

Batch Runner - для запуска из VScode

Как включить поддержку флоат для std вывода:
https://stackoverflow.com/questions/54534700/enabling-floating-point-emulation-in-gcc-arm-none-eabi

Частоту шины APB1 (SPI2) снизил в четыре раза, иначе были проблемы с передачей CC1101 (снижалась мощность передачи) 
(можно просто установить предделитель боадрейта в настройках SPI)

## Tools

### Compiler
*GCC* (gcc-arm-none-eabi) version 5.4.1 20160919

### Setting up the build system in WINDOWS10 

1. Download GNU Arm Embedded Toolchain 
https://developer.arm.com/downloads/-/gnu-rm#:~:text=The%20GNU%20Arm%20Embedded%20Toolchain,Arm%20Cortex%2DR%20processor%20families
or
https://launchpad.net/gcc-arm-embedded/+download

2. Download Windows Build Tools binaries (Make, cp, rm, echo, sh...)
https://github.com/xpack-dev-tools/windows-build-tools-xpack/releases/

3. Add paths to the PATH environment variable in Windows

Подробная статья: https://habr.com/ru/articles/673522/

### RTOS
Prototreads AD v1.4

https://dunkels.com/adam/pt/

### Code editor
*VS Code* file Pac-ManGame.code-workspace

## Build 
To make the release project , run **makeProject.bat** (OR *make -j* cmd command)

To make the debug project , run **makeProjectDebug.bat** (OR *make -j1 debug* cmd command)

To clean the project, run **makeClean.bat** (OR *make clean*)