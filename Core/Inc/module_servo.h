#ifndef __MODULE_SERVO_H__
#define __MODULE_SERVO_H__

#include "proto.h"

void Servo_Handle(const ProtoFrame *frame);
void Servo_Task(void);

#endif /* __MODULE_SERVO_H__ */
