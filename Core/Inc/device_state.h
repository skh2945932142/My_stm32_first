#ifndef __DEVICE_STATE_H__
#define __DEVICE_STATE_H__

#include <stdint.h>

typedef enum {
  RGB_MODE_OFF = 0,
  RGB_MODE_STATIC,
  RGB_MODE_BREATH
} RgbMode;

typedef struct {
  uint8_t rgb_r;
  uint8_t rgb_g;
  uint8_t rgb_b;
  RgbMode rgb_mode;
  uint16_t rgb_breath_period_ms;
  uint8_t relay_on;
  uint8_t buzzer_active;
  uint16_t buzzer_freq_hz;
  uint16_t buzzer_duration_ms;
  uint8_t servo_angle;
  uint8_t servo_sweep_enabled;
  uint8_t motor_speed;
  uint8_t motor_running;
  uint8_t sensor_auto_enabled;
  uint16_t sensor_interval_ms;
  float temperature_c;
  float humidity_pct;
  float distance_cm;
  uint16_t ntc_raw;
  uint16_t pot_raw;
  uint8_t key1_pressed;
  uint8_t key2_pressed;
  uint8_t encoder_button_pressed;
  int16_t encoder_position;
  uint8_t oled_inverted;
  char oled_text[32];
} DeviceState;

void DeviceState_Init(void);
DeviceState *DeviceState_GetMutable(void);
const DeviceState *DeviceState_Get(void);

#endif /* __DEVICE_STATE_H__ */
