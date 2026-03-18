#ifndef __MODULE_RGB_H__
#define __MODULE_RGB_H__

#include "proto.h"

#include <stdint.h>

void Rgb_Handle(const ProtoFrame *frame);
void Rgb_Task(void);
void Rgb_Set(uint8_t r, uint8_t g, uint8_t b);
void Rgb_Off(void);

#endif /* __MODULE_RGB_H__ */
