# RF_HACK

(EN)

A platform for testing wireless transmission and radio interfaces

Currently supports:

- Sub-GHz CC1101 TI transceiver
- GPS module GY-NEO-6M-V2
- 2.4GHz NRF24L01+ (under implementation)

Interaction with the software takes place through the ILI9341 display + xpt2046 touchscreen + encoder or CLI interface

Performs:

- reception with GPS coordinates display;
- transmission and reception in the Sub-GHz range of the test string;
- range scanning mode with spectrum output on the display;
- continuous generation of the test signal in the selected LPD channel (433MHz).

(РУС)

Платформа для тестирования беспроводной передачи и радиоинтерфейсов

На текущий момент поддерживает:

- Sub-GHz приёмопередатчик СС1101 TI
- GPS модуль GY-NEO-6M-V2
- 2.4 ГГц nrf24l01+ (в процессе реализации)

Взаимодействие с ПО происходит через дисплей ILI9341 + тачскрин xpt2046 + энкодер мли CLI.

Осуществляет:

- прием и отображение GPS-координат;
- передачу и прием в Sub-GHz диапазоне тестовой строки;
- режим сканирования диапазона с выводом спектра;
- непрерывную генерацю тестового сигнала в выбранном канале LPD (433 МГц).

<img src="https://github.com/sergey12malyshev/RF_HACK/blob/develop/shematic/photo_1.jpg" width=15% height=15%>  <img src="https://github.com/sergey12malyshev/RF_HACK/blob/develop/shematic/photo_2.jpg" width=20% height=20%> 

Video on youtube:

[![IMAGE ALT TEXT HERE](http://img.youtube.com/vi/krAb-4GRXaA/0.jpg)](http://www.youtube.com/watch?v=krAb-4GRXaA)


## MCU
STM32F401CC, High-performance access line, Arm Cortex-M4 core with DSP and FPU, 256 Kbytes of Flash memory, 84 MHz CPU, ART Accelerator

Batch Runner - to run from VSCode

How to enable float support for std withdrawal:
https://stackoverflow.com/questions/54534700/enabling-floating-point-emulation-in-gcc-arm-none-eabi

The frequency of the APB1 (SPI2) bus was reduced four times, otherwise there were problems with the transmission power of CC1101 (the frequency of exchange with CC1101 is no more than 10 Mhz)

## 🛠️ Tools

### Compiler
*GCC* (gcc-arm-none-eabi) version 5.4.1 20160919

Program language: C11 and C++11

#### Setting up the build system in WINDOWS10 

1. Download GNU Arm Embedded Toolchain 
https://developer.arm.com/downloads/-/gnu-rm#:~:text=The%20GNU%20Arm%20Embedded%20Toolchain,Arm%20Cortex%2DR%20processor%20families
or
https://launchpad.net/gcc-arm-embedded/+download

2. Download Windows Build Tools binaries (Make, cp, rm, echo, sh...)
https://github.com/xpack-dev-tools/windows-build-tools-xpack/releases/

3. Add paths to the PATH environment variable in Windows

Detailed article: https://habr.com/ru/articles/673522/

### RTOS
Prototreads AD v1.4

https://dunkels.com/adam/pt/

### Code editor
*VS Code* file Pac-ManGame.code-workspace

## ⚙️ Build 

The build can be started via VSCode Tasks

To make the release project run **makeProject.bat** (OR *make -j* cmd command)

To make the debug project run **makeProjectDebug.bat** (OR *make -j1 debug* cmd command)

To clean the project run **makeClean.bat** (OR *make clean*)

## 📥 Programm firmware
Connect ST-Link V2 to SWD connector. Run **programFlash.bat**

Connect J-Link to SWD connector. Run **programFlash_Jlink.cmd**

## 🔄 Update firmware use system bootloader (DFU Mode)

UART CLI-> BOOT command to launch the system bootloader. Connect to PC via USB Type C. Run **updateFirmware.cmd** or launch STM32CubeProgrammer (select the desired COM-port and download the firmware).

## 🖥️ Command Line Interface

USB-UART 115200 Baud rate 8N1

Terminal configuration file: utils\TERATERM.INI

Enter *help* command

## 🔍 Static code analyzer
Cppcheck 2.10 https://cppcheck.sourceforge.io/

Run **RunStaticAnalysisCODE.cmd**

It is possible to output the analysis result to a file

## 🧰 Hardware
- Black pill STM32F401CC
- CC1101 module
- GY-NEO-6M-V2 GPS module
- 2.8 TFT SPI 240x320 display (ILI9341)
- TP4056 charge module with protection
- Battery 18650 li-ion
- CH340N USB-UART module (Optional for updating firmware via USB Type-C and working with CLI)
- P-MOSFET for power switch

<img src="https://github.com/sergey12malyshev/RF_HACK/blob/develop/shematic/shematic.png" width=35% height=35%> 

## 🌿 About repo

Work in the repository is carried out through the Git-flow branching model

Run the **generateRelease.bat** to automatically generate an archive with the software 


## ⚖️⚠️ Legal Compliance / Ответственность

(EN)
This device operates on radio frequencies that may be regulated by local laws and international treaties.  
**It is your responsibility** to ensure that the use of this hardware and software complies with all applicable regulations in your country or region.

Key points to consider:
- **Frequency bands**: Ensure you are operating only in license‑free bands (e.g., LPD, PMR, ISM) or that you hold the necessary amateur radio license.
- **Output power**: Many bands (e.g., LPD 433 MHz) have strict power limits (typically ≤10 mW). Exceeding these limits may be illegal.
- **Spurious emissions**: The device must not cause harmful interference to other services.
- **Antenna restrictions**: Some regulations limit antenna type and gain.
- **Electromagnetic compatibility (EMC)**: The device should not disturb nearby electronic equipment.

The authors and contributors of this project **assume no liability** for any misuse or non‑compliance with local laws. By using this project, you agree that you are solely responsible for its lawful operation.

---

(РУС)
Данное устройство работает на радиочастотах, которые могут регулироваться местными законами и международными соглашениями.  
**Вы несёте полную ответственность** за то, чтобы использование этого оборудования и программного обеспечения соответствовало всем применимым нормам в вашей стране или регионе.

На что следует обратить особое внимание:
- **Частотные диапазоны**: Убедитесь, что вы работаете только в безлицензионных диапазонах и каналах (например, LPD, PMR, ISM) или имеете соответствующую радиолюбительскую категорию.
- **Выходная мощность**: Для многих диапазонов (например, LPD 433 МГц) установлены жёсткие ограничения по мощности (обычно ≤10 мВт). Превышение этих лимитов может быть незаконным.
- **Побочные излучения**: Устройство не должно создавать помех другим службам.
- **Ограничения на антенны**: Некоторые нормативы ограничивают тип и коэффициент усиления антенны.
- **Электромагнитная совместимость (ЭМС)**: Устройство не должно создавать помех близлежащей электронной аппаратуре.

Авторы и участники этого проекта **не несут ответственности** за любое неправомерное использование или несоблюдение местных законов. Используя этот проект, вы соглашаетесь, что только вы отвечаете за его законную эксплуатацию.
