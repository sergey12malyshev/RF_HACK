/*
 test.cpp - a test file for checking C++ compilation in the project
*/

#include <cstdint>
#include <stdbool.h>

#include "test.h"

class SimpleTest 
{
private:
    uint32_t value;
public:
    //constructor
    SimpleTest() : value(0) {}
    
    void set(uint32_t v) { value = v; }
    uint32_t get() const { return value; }
    uint32_t add(uint32_t x) { return value + x; }
};

// Global oject
static SimpleTest simpleTest;

// Реализация C-функций
extern "C" {
    
bool test_cpp_function(void) 
{
    const uint32_t value = 99;

    simpleTest.set(value);
    uint32_t result = simpleTest.get();

    return (bool)(value == result);
}

uint32_t test_cpp_add_numbers(uint32_t a, uint32_t b) 
{
    simpleTest.set(a);
    return simpleTest.add(b);
}

} // extern "C"