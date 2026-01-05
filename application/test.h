#pragma once
#ifndef TEST_H
#define TEST_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool test_cpp_function(void);
uint32_t test_cpp_add_numbers(uint32_t a, uint32_t b);

#ifdef __cplusplus
}
#endif

#endif // TEST_H