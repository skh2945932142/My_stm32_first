#ifndef __BOARD_HW_H__
#define __BOARD_HW_H__

#include "main.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint8_t key1_pressed;
  uint8_t key2_pressed;
  uint8_t encoder_button_pressed;
  int16_t encoder_position;
} BoardInputState;

#define BOARD_HW_ADC_CHANNEL_NTC 4U
#define BOARD_HW_ADC_CHANNEL_POT 5U

void BoardHw_Init(void);
void BoardHw_Task(void);

void BoardHw_SetRelay(uint8_t enabled);
uint8_t BoardHw_GetRelay(void);

void BoardHw_BuzzerBeep(uint16_t freq_hz, uint16_t duration_ms);
void BoardHw_BuzzerStop(void);
uint8_t BoardHw_IsBuzzerActive(void);

void BoardHw_SetServoAngle(uint8_t angle);

void BoardHw_SetMotorSpeed(uint8_t speed_percent);
void BoardHw_StopMotor(void);

bool BoardHw_ReadAht20(float *temperature_c, float *humidity_pct);
bool BoardHw_ReadUltrasonic(float *distance_cm);
uint16_t BoardHw_ReadAdcChannel(uint8_t channel);

void BoardHw_GetInputState(BoardInputState *state);

#endif /* __BOARD_HW_H__ */
