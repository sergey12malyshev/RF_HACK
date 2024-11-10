#ifndef __ENCODER_DRIVER_H
#define __ENCODER_DRIVER_H


bool encoder_getStateSwitch(void);
void encoder_setStateSwitch(bool const s);
void encoder_init(void);
void encoder_process(void);

#endif // __ENCODER_DRIVER_H