#pragma once
#ifndef POWER_H
#define POWER_H

#ifdef __cplusplus
extern "C" {
#endif

// C-совместимый интерфейс
void power_systemReset(void);
void power_wdtReset(void);

#ifdef __cplusplus
}
#endif

// C++ class
#ifdef __cplusplus

#include <cstdint>

class Power 
{
public:
    static void system_reset();
    static void wdt_reset();
private:
    // Auxiliary methods
    static void disableInterrupts();
    static void enableInterrupts();
};

#endif // __cplusplus

#endif // POWER_H