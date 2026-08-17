# PIR Motion Sensor and Reusable GPIO Helpers

This note explains the PIR motion sensor feature and the reusable GPIO helper macros used to configure STM32 GPIO pins more cleanly.

## Purpose

The PIR motion sensor adds motion detection to the security light system.

The system behaviour is:

```text
Bright environment           -> security light off
Dark + no motion             -> security light dimmed
Dark + motion detected       -> security light full brightness + warning LED on
```

The PIR sensor is connected to PA1 and is treated as a digital input.

```text
PIR output LOW   -> no motion detected
PIR output HIGH  -> motion detected
```

## Why the PIR Uses GPIO Input

The PIR module drives its output pin either HIGH or LOW. Because of this, the STM32 does not need to use ADC for the PIR sensor.

Unlike the light detector on PA0, which produces an analog voltage, the PIR output is already digital.

```text
LDR/light detector -> analog input -> ADC needed
PIR motion sensor  -> digital input -> GPIO input needed
```

## No Pull-up or Pull-down for First Test

For the first test, no internal pull-up or pull-down resistor is enabled.

Reason:

```text
The PIR module actively drives its output pin HIGH or LOW.
```

So the STM32 input should read the PIR output directly.

If the signal becomes unstable later, a pull-down can be enabled to give the input a defined LOW state when no valid signal is present.

## Why Reusable GPIO Helpers Were Added

GPIO registers such as `MODER` and `PUPDR` use 2 bits per pin.

For example:

```text
PA0 -> bits 1:0
PA1 -> bits 3:2
PA8 -> bits 17:16
```

Writing separate bit masks for every pin would make the code repetitive.

Instead, reusable helper macros calculate the correct bit position from the pin number.

## Generic GPIO Field Helpers

```c
#define GPIO_2BIT_FIELD_WIDTH      (2U)
#define GPIO_2BIT_MASK             (0x3U)

#define GPIO_PIN_SHIFT(pin)        ((pin) * GPIO_2BIT_FIELD_WIDTH)
#define GPIO_PIN_2BIT_MASK(pin)    (GPIO_2BIT_MASK << GPIO_PIN_SHIFT(pin))
#define GPIO_PIN_MASK(pin)         (1U << (pin))
```

## What the Helper Macros Mean

### `GPIO_PIN_SHIFT(pin)`

```c
#define GPIO_PIN_SHIFT(pin) ((pin) * GPIO_2BIT_FIELD_WIDTH)
```

This calculates where a pin's 2-bit field starts.

For PA1:

```text
pin = 1
shift = 1 * 2 = 2
```

So PA1 uses bits `3:2`.

For PA8:

```text
pin = 8
shift = 8 * 2 = 16
```

So PA8 uses bits `17:16`.

### `GPIO_PIN_2BIT_MASK(pin)`

```c
#define GPIO_PIN_2BIT_MASK(pin) (GPIO_2BIT_MASK << GPIO_PIN_SHIFT(pin))
```

This creates a 2-bit mask in the correct position for the chosen pin.

For PA1:

```text
0x3 << 2 = bits 3:2
```

This lets the code clear or update the full GPIO field safely.

### `GPIO_PIN_MASK(pin)`

```c
#define GPIO_PIN_MASK(pin) (1U << (pin))
```

This creates a normal 1-bit pin mask.

It is used for reading or writing pin states with `IDR` or `ODR`.

Example:

```c
#define PIR_SENSOR GPIO_PIN_MASK(PIR_SENSOR_PIN)
```

## GPIO Mode Values

```c
#define GPIO_MODE_INPUT            (0x0U)
#define GPIO_MODE_OUTPUT           (0x1U)
#define GPIO_MODE_ALTERNATE        (0x2U)
#define GPIO_MODE_ANALOG           (0x3U)
```

These values match the STM32 GPIO mode field:

```text
00 -> input mode
01 -> output mode
10 -> alternate function mode
11 -> analog mode
```

## GPIO Pull-up/Pull-down Values

```c
#define GPIO_PUPD_NONE             (0x0U)
#define GPIO_PUPD_PULLUP           (0x1U)
#define GPIO_PUPD_PULLDOWN         (0x2U)
```

These values match the STM32 pull-up/pull-down field:

```text
00 -> no pull-up, no pull-down
01 -> pull-up
10 -> pull-down
```

## Reusable GPIO Configuration Macros

```c
#define GPIO_SET_MODE(GPIOx_MODER, pin, mode)                  \
    do {                                                        \
        (GPIOx_MODER) &= ~GPIO_PIN_2BIT_MASK(pin);              \
        (GPIOx_MODER) |=  (((mode) & GPIO_2BIT_MASK)            \
                           << GPIO_PIN_SHIFT(pin));             \
    } while (0)

#define GPIO_SET_PUPD(GPIOx_PUPDR, pin, pupd)                  \
    do {                                                        \
        (GPIOx_PUPDR) &= ~GPIO_PIN_2BIT_MASK(pin);              \
        (GPIOx_PUPDR) |=  (((pupd) & GPIO_2BIT_MASK)            \
                           << GPIO_PIN_SHIFT(pin));             \
    } while (0)
```

## Why the Backslash Is Used

The backslash `\` allows a macro to continue onto the next line.

Without it, the macro would end after the first line of the `#define`.

This is needed because `GPIO_SET_MODE` and `GPIO_SET_PUPD` contain multiple statements.

## Why `do { } while (0)` Is Used

The `do { } while (0)` wrapper makes the macro behave like one normal C statement.

This means it can be used safely like this:

```c
GPIO_SET_MODE(GPIOA_MODER, PIR_SENSOR_PIN, GPIO_MODE_INPUT);
```

Even though the macro expands into multiple operations internally.

## Pin Numbers and Pin Masks

Pin numbers and pin masks are kept separate.

```c
#define LIGHT_DETECTOR_PIN         (0U)  // PA0
#define PIR_SENSOR_PIN             (1U)  // PA1
#define SECURITY_LIGHT_PIN         (4U)  // PB4
#define WARNING_LED_PIN            (8U)  // PA8
```

Pin numbers are used when configuring GPIO fields:

```c
GPIO_SET_MODE(GPIOA_MODER, PIR_SENSOR_PIN, GPIO_MODE_INPUT);
```

Pin masks are used when reading or writing pin states:

```c
#define PIR_SENSOR GPIO_PIN_MASK(PIR_SENSOR_PIN)
```

## PIR Initialisation

```c
void pir_sensor_init(void)
{
    /* Enable GPIOA clock */
    RCC_AHB1ENR |= GPIOAEN;

    /* Configure PA1 as input mode */
    GPIO_SET_MODE(GPIOA_MODER, PIR_SENSOR_PIN, GPIO_MODE_INPUT);

    /* No pull-up or pull-down for first test */
    GPIO_SET_PUPD(GPIOA_PUPDR, PIR_SENSOR_PIN, GPIO_PUPD_NONE);
}
```

## PIR Read Function

```c
uint8_t pir_motion_detected(void)
{
    return (GPIOA_IDR & PIR_SENSOR) != 0;
}
```

This checks whether PA1 is HIGH.

```text
HIGH -> motion detected
LOW  -> no motion detected
```

The `!= 0` converts the register check into a clean true/false result.

## Main Loop Behaviour

```c
while (1)
{
    if (adc_dma_value < NIGHT_THRESHOLD)
    {
        TIM3_CCR1 = 50;  // Dark: dim security light

        if (pir_motion_detected())
        {
            TIM3_CCR1 = 100;        // Motion detected: full brightness
            GPIOA_ODR |= WARNING_LED;
        }
        else
        {
            TIM3_CCR1 = 50;         // No motion: keep dimmed
            GPIOA_ODR &= ~WARNING_LED;
        }
    }
    else
    {
        TIM3_CCR1 = 0;              // Bright: light off
        GPIOA_ODR &= ~WARNING_LED;
    }
}
```

## Summary

The PIR feature adds digital motion detection using PA1. The reusable GPIO helpers make it easier to configure any GPIO pin without writing separate masks for each pin. The firmware now combines ADC DMA light detection, PIR motion detection, PWM brightness control, and warning LED output to create a more complete security monitoring behaviour.
