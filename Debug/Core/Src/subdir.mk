################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/app_blu.c \
../Core/Src/board_hw.c \
../Core/Src/device_state.c \
../Core/Src/font.c \
../Core/Src/gpio.c \
../Core/Src/main.c \
../Core/Src/module_adc.c \
../Core/Src/module_buzzer.c \
../Core/Src/module_motor.c \
../Core/Src/module_oled.c \
../Core/Src/module_relay.c \
../Core/Src/module_rgb.c \
../Core/Src/module_sensor.c \
../Core/Src/module_servo.c \
../Core/Src/module_sys.c \
../Core/Src/oled.c \
../Core/Src/proto.c \
../Core/Src/stm32f1xx_hal_msp.c \
../Core/Src/stm32f1xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f1xx.c \
../Core/Src/transport_uart.c 

OBJS += \
./Core/Src/app_blu.o \
./Core/Src/board_hw.o \
./Core/Src/device_state.o \
./Core/Src/font.o \
./Core/Src/gpio.o \
./Core/Src/main.o \
./Core/Src/module_adc.o \
./Core/Src/module_buzzer.o \
./Core/Src/module_motor.o \
./Core/Src/module_oled.o \
./Core/Src/module_relay.o \
./Core/Src/module_rgb.o \
./Core/Src/module_sensor.o \
./Core/Src/module_servo.o \
./Core/Src/module_sys.o \
./Core/Src/oled.o \
./Core/Src/proto.o \
./Core/Src/stm32f1xx_hal_msp.o \
./Core/Src/stm32f1xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f1xx.o \
./Core/Src/transport_uart.o 

C_DEPS += \
./Core/Src/app_blu.d \
./Core/Src/board_hw.d \
./Core/Src/device_state.d \
./Core/Src/font.d \
./Core/Src/gpio.d \
./Core/Src/main.d \
./Core/Src/module_adc.d \
./Core/Src/module_buzzer.d \
./Core/Src/module_motor.d \
./Core/Src/module_oled.d \
./Core/Src/module_relay.d \
./Core/Src/module_rgb.d \
./Core/Src/module_sensor.d \
./Core/Src/module_servo.d \
./Core/Src/module_sys.d \
./Core/Src/oled.d \
./Core/Src/proto.d \
./Core/Src/stm32f1xx_hal_msp.d \
./Core/Src/stm32f1xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f1xx.d \
./Core/Src/transport_uart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/app_blu.cyclo ./Core/Src/app_blu.d ./Core/Src/app_blu.o ./Core/Src/app_blu.su ./Core/Src/board_hw.cyclo ./Core/Src/board_hw.d ./Core/Src/board_hw.o ./Core/Src/board_hw.su ./Core/Src/device_state.cyclo ./Core/Src/device_state.d ./Core/Src/device_state.o ./Core/Src/device_state.su ./Core/Src/font.cyclo ./Core/Src/font.d ./Core/Src/font.o ./Core/Src/font.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/module_adc.cyclo ./Core/Src/module_adc.d ./Core/Src/module_adc.o ./Core/Src/module_adc.su ./Core/Src/module_buzzer.cyclo ./Core/Src/module_buzzer.d ./Core/Src/module_buzzer.o ./Core/Src/module_buzzer.su ./Core/Src/module_motor.cyclo ./Core/Src/module_motor.d ./Core/Src/module_motor.o ./Core/Src/module_motor.su ./Core/Src/module_oled.cyclo ./Core/Src/module_oled.d ./Core/Src/module_oled.o ./Core/Src/module_oled.su ./Core/Src/module_relay.cyclo ./Core/Src/module_relay.d ./Core/Src/module_relay.o ./Core/Src/module_relay.su ./Core/Src/module_rgb.cyclo ./Core/Src/module_rgb.d ./Core/Src/module_rgb.o ./Core/Src/module_rgb.su ./Core/Src/module_sensor.cyclo ./Core/Src/module_sensor.d ./Core/Src/module_sensor.o ./Core/Src/module_sensor.su ./Core/Src/module_servo.cyclo ./Core/Src/module_servo.d ./Core/Src/module_servo.o ./Core/Src/module_servo.su ./Core/Src/module_sys.cyclo ./Core/Src/module_sys.d ./Core/Src/module_sys.o ./Core/Src/module_sys.su ./Core/Src/oled.cyclo ./Core/Src/oled.d ./Core/Src/oled.o ./Core/Src/oled.su ./Core/Src/proto.cyclo ./Core/Src/proto.d ./Core/Src/proto.o ./Core/Src/proto.su ./Core/Src/stm32f1xx_hal_msp.cyclo ./Core/Src/stm32f1xx_hal_msp.d ./Core/Src/stm32f1xx_hal_msp.o ./Core/Src/stm32f1xx_hal_msp.su ./Core/Src/stm32f1xx_it.cyclo ./Core/Src/stm32f1xx_it.d ./Core/Src/stm32f1xx_it.o ./Core/Src/stm32f1xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f1xx.cyclo ./Core/Src/system_stm32f1xx.d ./Core/Src/system_stm32f1xx.o ./Core/Src/system_stm32f1xx.su ./Core/Src/transport_uart.cyclo ./Core/Src/transport_uart.d ./Core/Src/transport_uart.o ./Core/Src/transport_uart.su

.PHONY: clean-Core-2f-Src

