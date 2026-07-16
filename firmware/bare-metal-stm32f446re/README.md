# Bare-Metal STM32F446RE Firmware

This folder contains the bare-metal C implementation for the STM32 Smart Depot Safety and Access Monitoring System.

The purpose of this firmware version is to relearn STM32 embedded programming at register level instead of relying on HAL functions.

## Approach

This implementation avoids STM32 HAL functions such as:

- `HAL_GPIO_WritePin`
- `HAL_GPIO_ReadPin`
- `HAL_ADC_Start`
- `HAL_TIM_PWM_Start`
- `HAL_Delay`

Instead, the firmware uses direct register access 
