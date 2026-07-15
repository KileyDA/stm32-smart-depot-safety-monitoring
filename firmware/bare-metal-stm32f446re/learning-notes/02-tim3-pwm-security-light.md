# TIM3 PWM Implementation for Security Light

This note explains the reasoning behind using PWM to control the security light in the bare-metal STM32F446RE firmware.

## Purpose

The security light should support brightness control, not only simple ON/OFF control.

For a normal GPIO output, the CPU controls the pin directly by writing to the output data register:

```c
GPIOB_ODR |= SECURITY_LIGHT;   // ON
GPIOB_ODR &= ~SECURITY_LIGHT;  // OFF
```

For PWM, the pin is controlled by a timer peripheral instead. The timer switches the pin HIGH and LOW quickly, and the LED brightness depends on how long the signal stays HIGH during each cycle.

PWM stands for Pulse Width Modulation.

## Pin and Timer Choice

The security light is connected to PB4:

```c
#define SECURITY_LIGHT (1U << 4)   // PB4
```

PB4 can be mapped to TIM3 Channel 1:

```text
PB4 -> TIM3_CH1
```

Therefore, TIM3 Channel 1 is used to generate the PWM signal for the security light.

## GPIO Mode Reasoning

For GPIO output, the pin mode would be:

```text
01 = General purpose output mode
```

For PWM, PB4 must be configured as:

```text
10 = Alternate function mode
```

This is because the pin is no longer controlled manually by software. Instead, it is connected internally to the timer peripheral.

PB4 uses GPIO mode bits `9:8`.

To configure PB4 as alternate function mode:

```c
GPIOB_MODER &= ~(1U << 8);   // Clear bit 8
GPIOB_MODER |=  (1U << 9);   // Set bit 9
```

This sets PB4 mode bits to `10`.

## Alternate Function Selection

GPIO pins can connect to different internal peripherals using alternate functions.

For PB4 to work as TIM3 Channel 1, it must use:

```text
AF2
```

PB4 is pin 4, so it uses the low alternate function register:

```c
GPIOB_AFRL
```

Each alternate function field is 4 bits wide.

PB4 uses bits `19:16` in `GPIOB_AFRL`.

To select AF2 for PB4:

```c
GPIOB_AFRL &= ~(0xFU << 16);  // Clear PB4 alternate function bits
GPIOB_AFRL |=  (0x2U << 16);  // Set PB4 to AF2
```

`0xFU` is used because `0xF` is binary `1111`, which targets a 4-bit field. The `U` means the value is unsigned, which is preferred for register bit masks.

## Why TIM3 Is Used

TIM3 is a general-purpose timer that supports PWM output channels.

TIM3 is suitable here because:

- It can generate PWM using output compare mode.
- PB4 can be mapped to TIM3 Channel 1.
- It does not require the advanced timer `BDTR` main output enable step used by TIM1.

With TIM3, once the PWM is configured and started, the timer hardware generates the waveform automatically.

## Important TIM3 Registers

| Register | Purpose |
|---|---|
| `TIM3_PSC` | Prescaler. Slows down the timer clock. |
| `TIM3_ARR` | Auto-reload register. Sets the PWM period. |
| `TIM3_CCR1` | Capture/compare register 1. Sets the duty cycle for Channel 1. |
| `TIM3_CCMR1` | Capture/compare mode register 1. Configures Channel 1 as PWM mode. |
| `TIM3_CCER` | Capture/compare enable register. Enables Channel 1 output. |
| `TIM3_EGR` | Event generation register. Forces an update event so settings are loaded. |
| `TIM3_CR1` | Control register 1. Starts the timer. |

## PWM Frequency and Duty Cycle

The PWM frequency is controlled by the timer clock, prescaler, and auto-reload value.

The timer counts from `0` up to the value in `ARR`.

The value in `CCR1` decides how long the PWM output stays HIGH during each cycle.

Example:

```c
TIM3_ARR  = 100 - 1;
TIM3_CCR1 = 50;
```

This gives a 50% duty cycle.

If `ARR` gives 100 counts per PWM cycle:

```text
CCR1 = 0    -> 0% duty cycle
CCR1 = 25   -> 25% duty cycle
CCR1 = 50   -> 50% duty cycle
CCR1 = 75   -> 75% duty cycle
CCR1 = 100  -> 100% duty cycle
```

For an LED, the duty cycle controls the perceived brightness.

## Setup Sequence

The PWM configuration should follow this sequence:

```text
1. Enable GPIOB clock.
2. Configure PB4 as alternate function mode.
3. Select AF2 for PB4 in GPIOB_AFRL.
4. Enable TIM3 clock.
5. Set the TIM3 prescaler using TIM3_PSC.
6. Set the PWM period using TIM3_ARR.
7. Set the duty cycle using TIM3_CCR1.
8. Configure TIM3 Channel 1 as PWM mode using TIM3_CCMR1.
9. Enable TIM3 Channel 1 output using TIM3_CCER.
10. Generate an update event using TIM3_EGR.
11. Start TIM3 using TIM3_CR1.
```

## Key Difference from Normal GPIO Output

With normal GPIO output, the CPU manually changes the pin state:

```c
GPIOB_ODR ^= SECURITY_LIGHT;
```

With PWM, the timer controls the pin automatically.

The CPU only sets the PWM configuration and duty cycle. After that, the hardware timer generates the waveform.

This is better than software toggling because the PWM timing is handled by the timer peripheral, not by delay loops.

## SUMMARY

For the security light, I used PWM instead of normal GPIO output because PWM allows brightness control. The security light is connected to PB4, and PB4 can be mapped to TIM3 Channel 1 using alternate function AF2.

To do this, I first configure PB4 as an alternate function pin instead of a general-purpose output. Then I select AF2 in the GPIOB alternate function register so that PB4 is internally connected to TIM3_CH1.

After the pin is mapped to the timer, I configure TIM3. The prescaler controls how fast the timer counts, the auto-reload register controls the PWM period, and the capture/compare register controls the duty cycle. The capture/compare mode register sets Channel 1 to PWM mode, and the capture/compare enable register enables the channel output.

Once the timer is started, TIM3 generates the PWM signal automatically. This means the CPU does not need to manually toggle the pin, making the output more reliable and better suited for brightness control.
