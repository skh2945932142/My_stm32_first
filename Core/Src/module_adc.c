#include "module_adc.h"

#include "board_hw.h"
#include "device_state.h"

#include <stdio.h>
#include <string.h>

void Adc_Handle(const ProtoFrame *frame) {
  DeviceState *state = DeviceState_GetMutable();
  char line[80];

  if (strcmp(frame->action, "READ") == 0) {
    state->ntc_raw = BoardHw_ReadAdcChannel(BOARD_HW_ADC_CHANNEL_NTC);
    state->pot_raw = BoardHw_ReadAdcChannel(BOARD_HW_ADC_CHANNEL_POT);
    Proto_SendAckOk(frame->request_id);
    snprintf(line, sizeof(line), "ntc=%u;pot=%u", state->ntc_raw,
        state->pot_raw);
    Proto_SendStat("ADC", line);
    return;
  }

  Proto_SendAckErr(frame->request_id, "BAD_ACTION", "unknown adc action");
}
