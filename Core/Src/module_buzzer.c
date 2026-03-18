#include "module_buzzer.h"

#include "board_hw.h"
#include "device_state.h"

#include <stdio.h>
#include <string.h>

void Buzzer_Handle(const ProtoFrame *frame) {
  DeviceState *state = DeviceState_GetMutable();
  uint16_t freq = (uint16_t)Proto_PayloadGetInt(frame->payload, "freq", 1200);
  uint16_t duration =
      (uint16_t)Proto_PayloadGetInt(frame->payload, "duration", 300);
  char line[64];

  if (strcmp(frame->action, "BEEP") == 0) {
    BoardHw_BuzzerBeep(freq, duration);
    state->buzzer_active = 1U;
    state->buzzer_freq_hz = freq;
    state->buzzer_duration_ms = duration;
    Proto_SendAckOk(frame->request_id);
    snprintf(line, sizeof(line), "state=beep;freq=%u;duration=%u", freq,
        duration);
    Proto_SendStat("BUZZER", line);
    return;
  }

  if (strcmp(frame->action, "STOP") == 0) {
    BoardHw_BuzzerStop();
    state->buzzer_active = 0U;
    Proto_SendAckOk(frame->request_id);
    Proto_SendStat("BUZZER", "state=idle");
    return;
  }

  if (strcmp(frame->action, "MUSIC") == 0) {
    Proto_SendAckErr(frame->request_id, "UNSUPPORTED", "music disabled");
    return;
  }

  Proto_SendAckErr(frame->request_id, "BAD_ACTION", "unknown buzzer action");
}

void Buzzer_Task(void) {
  DeviceState *state = DeviceState_GetMutable();

  if ((!BoardHw_IsBuzzerActive()) && state->buzzer_active) {
    state->buzzer_active = 0U;
    Proto_SendStat("BUZZER", "state=idle");
  }
}
