#include "module_relay.h"

#include "board_hw.h"
#include "device_state.h"

#include <stdio.h>
#include <string.h>

void Relay_Handle(const ProtoFrame *frame) {
  DeviceState *state = DeviceState_GetMutable();
  uint8_t value = (uint8_t)Proto_PayloadGetInt(frame->payload, "value", 0);
  char line[32];

  if (strcmp(frame->action, "SET") == 0) {
    value = value ? 1U : 0U;
    BoardHw_SetRelay(value);
    state->relay_on = value;
    Proto_SendAckOk(frame->request_id);
    snprintf(line, sizeof(line), "value=%u", value);
    Proto_SendStat("RELAY", line);
    return;
  }

  Proto_SendAckErr(frame->request_id, "BAD_ACTION", "unknown relay action");
}
