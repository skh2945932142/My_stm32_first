#ifndef __MODULE_SENSOR_H__
#define __MODULE_SENSOR_H__

#include "proto.h"

void Sensor_Handle(const ProtoFrame *frame);
void Sensor_Task(void);

#endif /* __MODULE_SENSOR_H__ */
