# RCC, AHB, and Peripheral Clock Gating

This note explains how the STM32 uses the RCC, buses, and clock gates to allow the CPU to access and control peripherals.

---

## Power vs Clock

```text
Power = allows the hardware to physically exist
Clock = allows the hardware logic to operate
```

When the STM32 powers on, the peripherals are physically part of the chip and have power available. However, many peripherals are inactive because their clock gates are closed by default.

A peripheral may be powered, but if its clock is not enabled, its internal logic cannot properly operate or respond to register access.

---

## What the RCC Does

RCC stands for:

```text
Reset and Clock Control
```

The RCC is the central control block that manages clock signals for the microcontroller.

It controls which peripherals receive clock signals by opening or closing clock gates.

A clock gate is like a switch:

```text
Clock gate closed → peripheral does not receive clock ticks
Clock gate open   → peripheral receives clock ticks and can operate
```

For example, before using GPIOA, its clock must be enabled through the RCC:

```c
RCC_AHB1ENR |= (1U << 0);   // Enable GPIOA clock
```

This opens the clock gate for GPIOA.

---

## What the AHB Bus Does

AHB stands for:

```text
Advanced High-performance Bus
```

The AHB is one of the internal bus pathways that connects the CPU to memory and peripherals.

A useful analogy is:

```text
Buses       = streets/roads
Peripherals = houses/buildings connected to those roads
CPU         = person/vehicle trying to access the houses
RCC         = central control unit housing control panels
Clock gates = switches that allow clock signals into each house
Internal Peripheral logic = Machines in the house that need the switches turned on to operate
Clock enable register = control panels for different group of houses
```

The STM32 has different internal buses, including:

```text
AHB1 → GPIOA, GPIOB, GPIOC, DMA, etc.
APB1 → TIM2, TIM3, USART2, I2C, etc.
APB2 → ADC1, USART1, SPI1, SYSCFG, etc.
```

The RCC has different clock enable registers for peripherals on these buses:

```text
RCC_AHB1ENR → enables clocks for AHB1 peripherals
RCC_APB1ENR → enables clocks for APB1 peripherals
RCC_APB2ENR → enables clocks for APB2 peripherals
```

There is not a separate RCC for each peripheral. There is one RCC block with different clock enable registers.

---

## Why GPIO Needs a Clock

GPIO may seem simple, but it still contains internal logic and registers, such as:

```text
MODER   → selects input/output/alternate/analogue mode
ODR     → output data register
IDR     → input data register
PUPDR   → pull-up/pull-down configuration
OTYPER  → output type configuration
```

These registers and the GPIO bus interface need a clock signal to update and respond properly.

For example, to use PA8 as an output:

```text
PA8 belongs to GPIOA
GPIOA is connected to AHB1
GPIOA clock is enabled through RCC_AHB1ENR
Then GPIOA registers can be configured
Then PA8 can drive the warning LED
```

The sequence is:

```text
1. CPU writes to RCC_AHB1ENR
2. RCC opens the GPIOA clock gate
3. GPIOA receives clock ticks
4. CPU configures GPIOA_MODER
5. CPU writes to GPIOA_ODR
6. PA8 output changes state
```

Without enabling the GPIOA clock first, GPIOA may not properly respond to register reads and writes.

---

## Why Other Peripherals Need Clocks

Timers clearly need clocks because they count clock ticks.

However, other peripherals also need clocks because they contain registers, state machines, counters, shift registers, and control logic.

### ADC

The ADC needs a clock to control:

```text
sampling time
conversion steps
data register update
end-of-conversion flag
```

If the ADC clock is too slow, conversions take longer.  
If it is too fast, readings may become unreliable or violate the datasheet timing limits.

### UART

UART uses the peripheral clock to generate baud rate timing.

If the clock frequency is wrong, the baud rate calculation will be wrong, and serial communication may fail.

### SPI and I2C

SPI and I2C use clocked logic to shift data, generate timing, control start/stop conditions, and update status flags.

### Timers and PWM

Timers count clock ticks.

For PWM, the timer clock affects:

```text
PWM frequency
PWM period
PWM resolution
duty cycle timing
```

For example, the security light PWM on PB4 uses TIM3. Therefore, both GPIOB and TIM3 clocks are needed.

---

## How Clock Frequency Affects Peripherals

The clock frequency controls how fast a peripheral's internal logic operates.

Examples:

```text
GPIO   → affects how quickly register changes can propagate
ADC    → affects conversion timing
UART   → affects baud rate generation
TIMx   → affects counting speed and PWM frequency
SPI    → affects data transfer speed
I2C    → affects bus timing
```

For a simple LED output, clock frequency is not very noticeable because the LED only needs to turn ON or OFF.

For ADC, timers, PWM, and communication peripherals, clock frequency matters much more.

---

## Project Examples

For this project:

```text
PA8  → WARNING_LED          → GPIOA clock required
PA10 → ACK_BUTTON           → GPIOA clock required
PA1  → PIR_OUT              → GPIOA clock required
PA0  → LDR_ADC_A0           → GPIOA clock and ADC1 clock required
PB4  → SECURITY_LIGHT_PWM   → GPIOB clock and TIM3 clock required
PB5  → BUZZER_CTRL          → GPIOB clock required
```

---

## Summary

```text
Power allows the peripheral hardware to exist.
Clock allows the peripheral logic to operate.
RCC controls the clock gates.
AHB/APB buses are pathways between the CPU and peripherals.
Clock enable registers decide which peripherals receive clock signals.
```

A simple mental model:

```text
Buses are streets.
Peripherals are houses on those streets.
The CPU travels through the streets to access the houses.
The RCC is the central control unit.
Clock enable registers are control panels for different streets.
Clock gates are switches that allow or block clock signals.
```

The RCC controls the flow of clock signals, not the main power supply.
