# CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

project(RF_HACK C CXX ASM)

# Опции для debug сборки (аналог make debug)
option(DEBUG_MAIN "Enable debug main functions" OFF)
option(USE_FULL_ASSERT "Enable full assertions" OFF)

# Параметры CPU (как в Makefile)
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

# Базовые флаги (как в Makefile)
set(BASE_C_FLAGS "${CPU_FLAGS} -Wall -fdata-sections -ffunction-sections -u _printf_float -u _scanf_float")
set(BASE_CXX_FLAGS "${CPU_FLAGS} -Wall -fdata-sections -ffunction-sections -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit -std=c++11")
set(BASE_ASM_FLAGS "${CPU_FLAGS} -Wall -fdata-sections -ffunction-sections")

# Определяем флаги для разных типов сборки
# ВАЖНО: Release и Debug используют ОДИНАКОВУЮ оптимизацию -Og (как в оригинальном Makefile)
set(CMAKE_C_FLAGS_RELEASE "${BASE_C_FLAGS} -Og")
set(CMAKE_CXX_FLAGS_RELEASE "${BASE_CXX_FLAGS} -Og")
set(CMAKE_ASM_FLAGS_RELEASE "${BASE_ASM_FLAGS} -Og")

# Debug добавляет отладочные символы
set(CMAKE_C_FLAGS_DEBUG "${BASE_C_FLAGS} -Og -g -gdwarf-2")
set(CMAKE_CXX_FLAGS_DEBUG "${BASE_CXX_FLAGS} -Og -g -gdwarf-2")
set(CMAKE_ASM_FLAGS_DEBUG "${BASE_ASM_FLAGS} -Og -g -gdwarf-2")

# Устанавливаем базовые флаги
set(CMAKE_C_FLAGS "${BASE_C_FLAGS} -Og" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${BASE_CXX_FLAGS} -Og" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS "${BASE_ASM_FLAGS} -Og" CACHE STRING "" FORCE)

# Определения (всегда активные, как в Makefile)
add_compile_definitions(
    USE_FULL_LL_DRIVER
    STM32F401xC
    USE_HAL_DRIVER
)

# Добавляем defines для debug сборки
if(DEBUG_MAIN)
    add_compile_definitions(DEBUG_MAIN)
    message(STATUS ">>> DEBUG_MAIN is ENABLED (debug version) <<<")
endif()

if(USE_FULL_ASSERT)
    add_compile_definitions(USE_FULL_ASSERT)
    message(STATUS ">>> USE_FULL_ASSERT is ENABLED <<<")
endif()

# Поиск исходников (как в Makefile)
file(GLOB_RECURSE CORE_SOURCES "Core/Src/*.c")
file(GLOB_RECURSE DRIVERS_SOURCES "Drivers/STM32F4xx_HAL_Driver/Src/*.c")
file(GLOB_RECURSE APP_C_SOURCES "application/*.c")
file(GLOB_RECURSE APP_CPP_SOURCES "application/*.cpp")
file(GLOB ASM_SOURCES "startup_*.s")

# Объединяем
set(SOURCES
    ${CORE_SOURCES}
    ${DRIVERS_SOURCES}
    ${APP_C_SOURCES}
    ${APP_CPP_SOURCES}
    ${ASM_SOURCES}
)

# Добавляем исполняемый файл (ЦЕЛЬ должна быть создана ДО target_include_directories)
add_executable(${PROJECT_NAME} ${SOURCES})

# Include директории (ПОСЛЕ создания цели)
target_include_directories(${PROJECT_NAME} PRIVATE
    Core/Inc
    application
    application/Display
    application/XPT2046
    application/GPS
    application/Pt_1.4
    application/CC1101
    Drivers/STM32F4xx_HAL_Driver/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    Drivers/CMSIS/Device/ST/STM32F4xx/Include
    Drivers/CMSIS/Include
)

# Устанавливаем язык для ассемблера
set_source_files_properties(${ASM_SOURCES} PROPERTIES LANGUAGE ASM)

# Скрипт линковщика
set(LD_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/STM32F401CCUx_FLASH.ld)

# Линкер опции (как в Makefile)
set(CMAKE_EXE_LINKER_FLAGS "${CPU_FLAGS} -specs=nosys.specs -specs=nano.specs -u _printf_float -u _scanf_float")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map,--cref")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections,--print-memory-usage -T ${LD_SCRIPT}")

# Библиотеки
link_libraries(c m nosys stdc++)

# Пост-обработка
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary -S $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
    COMMENT "Creating hex and bin files, printing size"
)