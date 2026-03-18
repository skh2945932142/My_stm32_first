#include "device_state.h"

#include <string.h>

static DeviceState s_device_state;

void DeviceState_Init(void) {
  memset(&s_device_state, 0, sizeof(s_device_state));
  s_device_state.rgb_mode = RGB_MODE_OFF;
  s_device_state.rgb_breath_period_ms = 1600U;
  s_device_state.servo_angle = 90U;
  s_device_state.sensor_interval_ms = 1000U;
}

DeviceState *DeviceState_GetMutable(void) {
  return &s_device_state;
}

const DeviceState *DeviceState_Get(void) {
  return &s_device_state;
}
