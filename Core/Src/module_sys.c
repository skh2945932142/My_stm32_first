#include "module_sys.h"

#include "board_hw.h"
#include "device_state.h"
#include "module_buzzer.h"
#include "module_motor.h"
#include "module_oled.h"
#include "module_relay.h"
#include "module_rgb.h"
#include "module_sensor.h"
#include "module_servo.h"

#include <stdio.h>
#include <string.h>

#define DEVICE_BOARD_NAME "keysking-stm32"
#define DEVICE_FW_VERSION "1.0.0"
#define DEVICE_TRANSPORT_NAME "serial"

void Sys_Handle(const ProtoFrame *frame) {
  char line[64];
  DeviceState *state;

  if (strcmp(frame->action, "HELLO") == 0) {
    state = DeviceState_GetMutable();

    Proto_SendAckOk(frame->request_id);
    Proto_SendStat("SYS",
        "board=" DEVICE_BOARD_NAME ";fw=" DEVICE_FW_VERSION ";transport="
            DEVICE_TRANSPORT_NAME);
    Proto_SendAllCapabilities();

    snprintf(line, sizeof(line), "r=%u;g=%u;b=%u", state->rgb_r, state->rgb_g,
        state->rgb_b);
    Proto_SendStat("RGB", line);

    snprintf(line, sizeof(line), "mode=ready;invert=%u",
        (unsigned int)state->oled_inverted);
    Proto_SendStat("OLED", line);

    snprintf(line, sizeof(line), "state=%s", state->buzzer_active ? "beep" : "idle");
    Proto_SendStat("BUZZER", line);

    snprintf(line, sizeof(line), "value=%u", state->relay_on);
    Proto_SendStat("RELAY", line);

    snprintf(line, sizeof(line), "angle=%u", state->servo_angle);
    Proto_SendStat("SERVO", line);

    if (state->motor_running) {
      snprintf(line, sizeof(line), "state=running;speed=%u", state->motor_speed);
    } else {
      snprintf(line, sizeof(line), "state=stopped;speed=%u", state->motor_speed);
    }
    Proto_SendStat("MOTOR", line);

    snprintf(line, sizeof(line), "auto=%u;interval=%u", state->sensor_auto_enabled,
        state->sensor_interval_ms);
    Proto_SendStat("SENSOR", line);

    snprintf(line, sizeof(line), "ntc=%u;pot=%u", state->ntc_raw, state->pot_raw);
    Proto_SendStat("ADC", line);
    return;
  }

  if (strcmp(frame->action, "PING") == 0) {
    Proto_SendAckOk(frame->request_id);
    Proto_SendStat("SYS", "alive=1");
    return;
  }

  if (strcmp(frame->action, "SYNC_TIME") == 0) {
    Proto_SendAckErr(frame->request_id, "UNSUPPORTED", "sync time disabled");
    return;
  }

  if (strcmp(frame->action, "REBOOT") == 0) {
    Proto_SendAckErr(frame->request_id, "UNSUPPORTED", "reboot disabled");
    return;
  }

  if (strcmp(frame->action, "ALL_OFF") == 0) {
    state = DeviceState_GetMutable();

    Rgb_Off();
    Oled_ClearScreen();
    BoardHw_BuzzerStop();
    BoardHw_SetRelay(0U);
    BoardHw_StopMotor();
    state->buzzer_active = 0U;
    state->relay_on = 0U;
    state->motor_speed = 0U;
    state->motor_running = 0U;
    state->servo_sweep_enabled = 0U;
    state->sensor_auto_enabled = 0U;

    Proto_SendAckOk(frame->request_id);
    Proto_SendStat("RGB", "r=0;g=0;b=0");
    Proto_SendStat("OLED", "mode=clear");
    Proto_SendStat("BUZZER", "state=idle");
    Proto_SendStat("RELAY", "value=0");
    Proto_SendStat("MOTOR", "state=stopped;speed=0");
    snprintf(line, sizeof(line), "angle=%u", state->servo_angle);
    Proto_SendStat("SERVO", line);
    snprintf(line, sizeof(line), "auto=0;interval=%u", state->sensor_interval_ms);
    Proto_SendStat("SENSOR", line);
    return;
  }

  Proto_SendAckErr(frame->request_id, "BAD_ACTION", "unknown sys action");
}
