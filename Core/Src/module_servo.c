#include "module_servo.h"

#include "board_hw.h"
#include "device_state.h"

#include <stdio.h>
#include <string.h>

typedef struct {
  uint8_t min_angle;
  uint8_t max_angle;
  uint8_t step;
  int8_t direction;
  uint32_t next_tick;
} ServoSweepState;

static ServoSweepState s_servo_sweep;

static void servo_apply_angle(uint8_t angle) {
  DeviceState *state = DeviceState_GetMutable();

  if (angle > 180U) {
    angle = 180U;
  }

  state->servo_angle = angle;
  BoardHw_SetServoAngle(angle);
}

void Servo_Handle(const ProtoFrame *frame) {
  char line[48];

  if (strcmp(frame->action, "ANGLE") == 0) {
    servo_apply_angle((uint8_t)Proto_PayloadGetInt(frame->payload, "value", 90));
    DeviceState_GetMutable()->servo_sweep_enabled = 0U;
    Proto_SendAckOk(frame->request_id);
    snprintf(line, sizeof(line), "angle=%u", DeviceState_Get()->servo_angle);
    Proto_SendStat("SERVO", line);
    return;
  }

  if (strcmp(frame->action, "SWEEP") == 0) {
    s_servo_sweep.min_angle =
        (uint8_t)Proto_PayloadGetInt(frame->payload, "min", 0);
    s_servo_sweep.max_angle =
        (uint8_t)Proto_PayloadGetInt(frame->payload, "max", 180);
    s_servo_sweep.step =
        (uint8_t)Proto_PayloadGetInt(frame->payload, "step", 10);
    if (s_servo_sweep.step == 0U) {
      s_servo_sweep.step = 10U;
    }
    s_servo_sweep.direction = 1;
    s_servo_sweep.next_tick = HAL_GetTick();
    DeviceState_GetMutable()->servo_sweep_enabled = 1U;
    servo_apply_angle(s_servo_sweep.min_angle);
    Proto_SendAckOk(frame->request_id);
    Proto_SendStat("SERVO", "state=sweep");
    return;
  }

  if (strcmp(frame->action, "STOP") == 0) {
    DeviceState_GetMutable()->servo_sweep_enabled = 0U;
    Proto_SendAckOk(frame->request_id);
    snprintf(line, sizeof(line), "angle=%u", DeviceState_Get()->servo_angle);
    Proto_SendStat("SERVO", line);
    return;
  }

  Proto_SendAckErr(frame->request_id, "BAD_ACTION", "unknown servo action");
}

void Servo_Task(void) {
  DeviceState *state = DeviceState_GetMutable();

  if (!state->servo_sweep_enabled) {
    return;
  }

  if ((int32_t)(HAL_GetTick() - s_servo_sweep.next_tick) < 0) {
    return;
  }

  s_servo_sweep.next_tick = HAL_GetTick() + 120U;

  if (s_servo_sweep.direction > 0) {
    if ((uint16_t)state->servo_angle + s_servo_sweep.step >= s_servo_sweep.max_angle) {
      servo_apply_angle(s_servo_sweep.max_angle);
      s_servo_sweep.direction = -1;
    } else {
      servo_apply_angle((uint8_t)(state->servo_angle + s_servo_sweep.step));
    }
  } else if (state->servo_angle <= (uint8_t)(s_servo_sweep.min_angle + s_servo_sweep.step)) {
    servo_apply_angle(s_servo_sweep.min_angle);
    s_servo_sweep.direction = 1;
  } else {
    servo_apply_angle((uint8_t)(state->servo_angle - s_servo_sweep.step));
  }
}
