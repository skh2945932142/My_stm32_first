#ifndef __MODULE_BUZZER_H__
#define __MODULE_BUZZER_H__

#include "proto.h"

void Buzzer_Handle(const ProtoFrame *frame);
void Buzzer_Task(void);

#endif /* __MODULE_BUZZER_H__ */
