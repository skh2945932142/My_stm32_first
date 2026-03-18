#ifndef __MODULE_OLED_H__
#define __MODULE_OLED_H__

#include "proto.h"

void Oled_ModuleInit(void);
void Oled_Handle(const ProtoFrame *frame);
void Oled_ClearScreen(void);

#endif /* __MODULE_OLED_H__ */
