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

## Register Access Approach

The firmware uses memory-mapped register access instead of STM32 HAL functions.

A helper macro is used to make register access cleaner:

```c
#define REG32(addr) (*(volatile uint32_t *)(addr))
```

This converts a register address into a volatile 32-bit register access.

For example:

```c
#define TIM3_CR1 REG32(TIM3_BASE + TIM3_CR1_OFFSET)
```

This means `TIM3_CR1` refers to the actual hardware register at that address.

`volatile` is required because hardware registers can change outside normal program flow, and the compiler must not optimise away those reads or writes.

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

To improve readability, the bit positions are defined as macros:

```c
#define PB4_MODE_BIT0 (1U << 8)
#define PB4_MODE_BIT1 (1U << 9)
```

To configure PB4 as alternate function mode:

```c
GPIOB_MODER &= ~PB4_MODE_BIT0;
GPIOB_MODER |=  PB4_MODE_BIT1;
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

The following macros make the mapping clearer:

```c
#define PB4_AFRL_SHIFT (16U)
#define GPIO_AF2_TIM3  (0x2U)
#define GPIO_AFR_MASK  (0xFU)
```

To select AF2 for PB4:

```c
GPIOB_AFRL &= ~(GPIO_AFR_MASK << PB4_AFRL_SHIFT);
GPIOB_AFRL |=  (GPIO_AF2_TIM3 << PB4_AFRL_SHIFT);
```

`0xFU` is used because `0xF` is binary `1111`, which targets a 4-bit field. The `U` means the value is unsigned, which is preferred for register bit masks.

Using `(0x2U << 16)` is clearer than only setting bit 17 because alternate function selection is a 4-bit field, not a single-bit enable.

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
| `TIM3_CCMR1` | Capture/compare mode register 1. Configures Channel 1 as output and PWM mode. |
| `TIM3_CCER` | Capture/compare enable register. Enables Channel 1 output. |
| `TIM3_EGR` | Event generation register. Forces an update event so settings are loaded. |
| `TIM3_CNT` | Counter register. Stores the current timer count. |
| `TIM3_CR1` | Control register 1. Enables preload and starts the timer. |

## Important TIM3 Bit Fields

| Macro | Purpose |
|---|---|
| `TIM_CR1_CEN` | Starts the timer counter. |
| `TIM_CR1_ARPE` | Enables auto-reload preload. |
| `TIM_EGR_UG` | Generates an update event. |
| `TIM_CCMR1_CC1S_MASK` | Clears Channel 1 selection bits so Channel 1 works as output. |
| `TIM_CCMR1_OC1PE` | Enables preload for Channel 1 duty cycle updates. |
| `TIM_CCMR1_OC1M_PWM1` | Selects PWM mode 1 for Channel 1. |
| `TIM_CCMR1_OC1M_MASK` | Clears the output compare mode bits before selecting PWM mode. |
| `TIM_CCER_CC1E` | Enables TIM3 Channel 1 output. |

## PWM Frequency and Duty Cycle

The PWM frequency is controlled by the timer clock, prescaler, and auto-reload value.

The timer counts from `0` up to the value in `ARR`.

The value in `CCR1` decides how long the PWM output stays HIGH during each cycle.

Current setup:

```c
TIM3_PSC  = 160 - 1;
TIM3_ARR  = 100 - 1;
TIM3_CCR1 = 50;
```

Assuming TIM3 receives a 16 MHz timer clock:

```text
Timer counter frequency = 16 MHz / 160
                        = 100 kHz

PWM frequency = 100 kHz / 100
              = 1 kHz
```

The duty cycle is:

```text
Duty cycle = CCR1 / ARR period count
           = 50 / 100
           = 50%
```

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

The PWM configuration follows this sequence:

```text
1. Enable GPIOB clock.
2. Configure PB4 as alternate function mode.
3. Select AF2 for PB4 in GPIOB_AFRL.
4. Enable TIM3 clock.
5. Set the TIM3 prescaler using TIM3_PSC.
6. Set the PWM period using TIM3_ARR.
7. Set the duty cycle using TIM3_CCR1.
8. Configure TIM3 Channel 1 as output using TIM3_CCMR1.
9. Configure TIM3 Channel 1 as PWM mode 1 using TIM3_CCMR1.
10. Enable preload for Channel 1 using TIM3_CCMR1.
11. Enable TIM3 Channel 1 output using TIM3_CCER.
12. Enable auto-reload preload using TIM3_CR1.
13. Generate an update event using TIM3_EGR.
14. Reset the counter using TIM3_CNT.
15. Start TIM3 using TIM3_CR1.
```

## Key Configuration Code

```c
void security_light_pwm_init(void)
{
    /* 1. Enable GPIOB clock */
    RCC_AHB1ENR |= GPIOBEN;

    /* 2. Configure PB4 as alternate function mode */
    GPIOB_MODER &= ~PB4_MODE_BIT0;
    GPIOB_MODER |=  PB4_MODE_BIT1;

    /* 3. Select AF2 for PB4: PB4 -> TIM3_CH1 */
    GPIOB_AFRL &= ~(GPIO_AFR_MASK << PB4_AFRL_SHIFT);
    GPIOB_AFRL |=  (GPIO_AF2_TIM3 << PB4_AFRL_SHIFT);

    /* 4. Enable TIM3 clock */
    RCC_APB1ENR |= TIM3EN;

    /* 5. Set TIM3 prescaler */
    TIM3_PSC = 160 - 1;

    /* 6. Set PWM period */
    TIM3_ARR = 100 - 1;

    /* 7. Set duty cycle */
    TIM3_CCR1 = 50;

    /* 8. Configure Channel 1 as output and PWM mode 1 */
    TIM3_CCMR1 &= ~TIM_CCMR1_CC1S_MASK;
    TIM3_CCMR1 &= ~TIM_CCMR1_OC1M_MASK;
    TIM3_CCMR1 |=  TIM_CCMR1_OC1M_PWM1;

    /* 9. Enable preload for Channel 1 */
    TIM3_CCMR1 |= TIM_CCMR1_OC1PE;

    /* 10. Enable TIM3 Channel 1 output */
    TIM3_CCER |= TIM_CCER_CC1E;

    /* 11. Enable auto-reload preload */
    TIM3_CR1 |= TIM_CR1_ARPE;

    /* 12. Generate update event */
    TIM3_EGR |= TIM_EGR_UG;

    /* 13. Reset counter */
    TIM3_CNT = 0;

    /* 14. Start TIM3 */
    TIM3_CR1 |= TIM_CR1_CEN;
}
```

## Key Difference from Normal GPIO Output

With normal GPIO output, the CPU manually changes the pin state:

```c
GPIOB_ODR ^= SECURITY_LIGHT;
```

With PWM, the timer controls the pin automatically.

The CPU only sets the PWM configuration and duty cycle. After that, the hardware timer generates the waveform.

This is better than software toggling because the PWM timing is handled by the timer peripheral, not by delay loops.

## Summary

For the security light, I used PWM instead of normal GPIO output because PWM allows brightness control. The security light is connected to PB4, and PB4 can be mapped to TIM3 Channel 1 using alternate function AF2.

To do this, I first configure PB4 as an alternate function pin instead of a general-purpose output. Then I select AF2 in the GPIOB alternate function register so that PB4 is internally connected to TIM3_CH1.

After the pin is mapped to the timer, I configure TIM3. The prescaler controls how fast the timer counts, the auto-reload register controls the PWM period, and the capture/compare register controls the duty cycle. The capture/compare mode register configures Channel 1 as an output and selects PWM mode 1, while the capture/compare enable register enables the channel output.

I also generate an update event so the timer loads the new settings, reset the counter to start from a known value, and then enable the timer. Once TIM3 is running, it generates the PWM signal automatically. This means the CPU does not need to manually toggle the pin, making the output more reliable and better suited for brightness control.
