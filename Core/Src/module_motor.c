#include "module_motor.h"

#include "board_hw.h"
#include "device_state.h"

#include <stdio.h>
#include <string.h>

void Motor_Handle(const ProtoFrame *frame) {
  DeviceState *state = DeviceState_GetMutable();
  uint8_t speed;
  char line[48];

  if (strcmp(frame->action, "RUN") == 0) {
    speed = (uint8_t)Proto_PayloadGetInt(frame->payload, "speed", 50);
    if (speed > 100U) {
      speed = 100U;
    }
    BoardHw_SetMotorSpeed(speed);
    state->motor_speed = speed;
    state->motor_running = (speed > 0U) ? 1U : 0U;
    Proto_SendAckOk(frame->request_id);
    snprintf(line, sizeof(line), "state=running;speed=%u", speed);
    Proto_SendStat("MOTOR", line);
    return;
  }

  if (strcmp(frame->action, "STOP") == 0) {
    BoardHw_StopMotor();
    state->motor_speed = 0U;
    state->motor_running = 0U;
    Proto_SendAckOk(frame->request_id);
    Proto_SendStat("MOTOR", "state=stopped;speed=0");
    return;
  }

  Proto_SendAckErr(frame->request_id, "BAD_ACTION", "unknown motor action");
}
