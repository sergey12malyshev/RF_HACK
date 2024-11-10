#ifndef __ENCODER_DRIVER_H
#define __ENCODER_DRIVER_H


bool encoder_getStateSwitch(void);
void encoder_setStateSwitch(bool const s);
void encoder_init(void);
void encoder_process(void);

int32_t encoder_getRotaryNum(void);
void encoder_setRotaryNum(int32_t);

#endif // __ENCODER_DRIVER_H