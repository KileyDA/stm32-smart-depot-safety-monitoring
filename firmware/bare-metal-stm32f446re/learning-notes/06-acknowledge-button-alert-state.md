# Acknowledge Button and Latched Alert State

This note explains the acknowledge button feature used to silence a motion alert while returning the system to the correct monitoring mode.

## Purpose

The button is used as an application-level acknowledge control.

It does not reset the microcontroller. Instead, it silences the current alert state.

```text
Dark + no motion             -> security light dimmed
Dark + motion detected       -> alert latches active
Button pressed during alert  -> alert silenced
Bright environment           -> system off/reset
```

## Button Hardware Behaviour

The acknowledge button is connected to D2 / PA10.

The circuit uses the STM32 internal pull-up resistor.

```text
Not pressed -> PA10 is pulled HIGH internally
Pressed     -> PA10 is connected to GND -> LOW
```

This means the button is active-low.

## Pin Definitions

The reusable GPIO helper macros separate pin numbers from pin masks.

```c
#define ACK_BUTTON_PIN (10U)  // PA10 / D2
#define ACK_BUTTON     GPIO_PIN_MASK(ACK_BUTTON_PIN)
```

The pin number is used for configuration.

The pin mask is used when reading or writing the GPIO register.

## Button Initialisation

```c
void ack_button_init(void)
{
    /* Enable GPIOA clock */
    RCC_AHB1ENR |= GPIOAEN;

    /* Configure PA10 as input mode */
    GPIO_SET_MODE(GPIOA_MODER, ACK_BUTTON_PIN, GPIO_MODE_INPUT);

    /* Enable internal pull-up resistor */
    GPIO_SET_PUPD(GPIOA_PUPDR, ACK_BUTTON_PIN, GPIO_PUPD_PULLUP);
}
```

The pull-up gives PA10 a defined HIGH state when the button is not pressed.

Without the pull-up, the input could float and produce unreliable readings.

## Button Read Function

```c
uint8_t ack_button_pressed(void)
{
    return (GPIOA_IDR & ACK_BUTTON) == 0;
}
```

The function returns true when the input is LOW because the button is active-low.

```text
GPIOA_IDR & ACK_BUTTON != 0 -> not pressed
GPIOA_IDR & ACK_BUTTON == 0 -> pressed
```

## Why a Latched Alert Is Needed

The PIR sensor output can return LOW after motion is no longer detected.

If the warning LED directly follows the PIR signal, the alert turns off immediately when PIR goes LOW.

For a security system, this is not ideal.

Instead, the system uses a latched alert state.

```text
Motion detected once -> alert stays active
Button pressed       -> alert is acknowledged/silenced
```

## State Variables

```c
uint8_t alert_active = 0;
uint8_t alert_acknowledged = 0;
```

`alert_active` stores whether the current alert is active.

`alert_acknowledged` stores whether the current alert has already been silenced by the button.

## Clean Main Loop Logic

Each input is read once per loop.

```c
uint8_t is_dark = adc_dma_value < NIGHT_THRESHOLD;
uint8_t motion_detected = pir_motion_detected();
uint8_t button_pressed = ack_button_pressed();
```

This avoids reading the same GPIO input multiple times during one decision cycle.

## Acknowledged Mode Behaviour

```c
if (is_dark)
{
    if (motion_detected && !alert_acknowledged)
    {
        alert_active = 1;
    }

    if (button_pressed && alert_active)
    {
        alert_active = 0;
        alert_acknowledged = 1;
    }

    if (!motion_detected)
    {
        alert_acknowledged = 0;
    }

    if (alert_active)
    {
        TIM3_CCR1 = 100;
        GPIOA_ODR |= WARNING_LED;
    }
    else
    {
        TIM3_CCR1 = 50;
        GPIOA_ODR &= ~WARNING_LED;
    }
}
else
{
    TIM3_CCR1 = 0;
    GPIOA_ODR &= ~WARNING_LED;

    alert_active = 0;
    alert_acknowledged = 0;
}
```

## Final Behaviour

```text
Bright:
Security light off.
Warning LED off.
Alert state reset.

Dark + no motion:
Security light dimmed at 50%.
Warning LED off.

Dark + motion:
Alert latches active.
Security light increases to 100%.
Warning LED turns on.

Dark + motion + button pressed:
Alert is acknowledged.
Warning LED turns off.
Security light returns to 50% monitoring mode.

Motion clears:
Acknowledgement resets so the next motion event can trigger a new alert.
```

## Summary

I added an acknowledge button connected to PA10 using the STM32 internal pull-up resistor. Because the button connects the pin to ground when pressed, the input is active-low, so the read function checks whether the PA10 bit in `GPIOA_IDR` is zero.

The button is used to silence a latched motion alert. When motion is detected in darkness, the alert state is stored in `alert_active`, so the warning does not disappear immediately when the PIR output changes. Pressing the acknowledge button clears the active alert and sets `alert_acknowledged`, returning the system to dim monitoring mode. The acknowledgement resets when motion clears or when the system leaves dark mode.
