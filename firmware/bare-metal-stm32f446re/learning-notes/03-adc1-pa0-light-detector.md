# ADC1 PA0 Light Detector Implementation

This note explains the ADC setup used to read the light detector/LDR input in the bare-metal STM32F446RE firmware.

## Purpose

The light detector provides an analog voltage that changes depending on the light level.

The STM32 cannot directly use an analog voltage in software, so ADC1 is used to convert the voltage into a digital value.

For a 12-bit ADC result:

```text
0V      -> approximately 0
3.3V    -> approximately 4095
```

The firmware uses this ADC value to decide whether the environment is dark enough to switch on the security light PWM output.

## Pin and ADC Channel Choice

The light detector is connected to PA0:

```c
#define LIGHT_DETECTOR (1U << 0) // PA0
```

PA0 maps to ADC1 Channel 0:

```text
PA0 -> ADC1_IN0 -> ADC channel 0
```

Therefore, ADC1 Channel 0 is used to read the light detector.

## Register Access Approach

The firmware uses direct memory-mapped register access instead of STM32 HAL functions.

A helper macro is used to access hardware registers:

```c
#define REG32(addr) (*(volatile uint32_t *)(addr))
```

This converts a register address into a writable/readable 32-bit volatile register.

`volatile` is required because hardware registers can change outside normal program flow, and the compiler must not optimise away the reads and writes.

## ADC Bus and Clock Reasoning

ADC1 is connected to the APB2 bus.

The APB2 base address is defined as:

```c
#define APB2_OFFSET (0x00010000UL)
#define APB2_BASE   (PERIPH_BASE + APB2_OFFSET)
```

ADC1 base address is then calculated from APB2:

```c
#define ADC1_OFFSET (0x00002000UL)
#define ADC1_BASE   (APB2_BASE + ADC1_OFFSET)
```

Before ADC1 can be configured, its peripheral clock must be enabled through RCC:

```c
RCC_APB2ENR |= ADC1EN;
```

`ADC1EN` enables the ADC1 peripheral clock.

## GPIO Analog Mode Reasoning

Before ADC1 can read PA0, the GPIO pin must be configured in analog mode.

For GPIO mode bits:

```text
00 = Input
01 = General purpose output
10 = Alternate function
11 = Analog mode
```

PA0 uses mode bits `1:0`.

The code defines:

```c
#define PA0_MODE_BIT0 (1U << 0)
#define PA0_MODE_BIT1 (1U << 1)
```

Then configures PA0 as analog mode:

```c
GPIOA_MODER |= PA0_MODE_BIT0;
GPIOA_MODER |= PA0_MODE_BIT1;
```

This sets PA0 mode bits to `11`.

Analog mode is required because the ADC needs direct access to the analog voltage on the pin.

## Important ADC1 Registers

| Register | Purpose |
|---|---|
| `ADC1_SR` | Status register. Holds flags such as end of conversion. |
| `ADC1_CR1` | Control register 1. Used for settings such as resolution, scan mode, and interrupts. |
| `ADC1_CR2` | Control register 2. Enables ADC, starts conversion, and controls conversion mode. |
| `ADC1_SMPR2` | Sample time register 2. Sets sampling time for ADC channels 0 to 9. |
| `ADC1_SQR1` | Regular sequence register 1. Sets the number of conversions in the sequence. |
| `ADC1_SQR3` | Regular sequence register 3. Selects the first channels in the conversion sequence. |
| `ADC1_DR` | Data register. Stores the ADC conversion result. |

## ADC Sequence Length

The ADC can convert one channel or multiple channels in a regular sequence.

This project currently reads one analog input, so the sequence length is one conversion.

STM32 stores the sequence length as:

```text
number of conversions - 1
```

Therefore:

```text
1 conversion -> write 0
2 conversions -> write 1
3 conversions -> write 2
```

The code defines:

```c
#define ADC_SQR1_LENGTH_SHIFT               (20U)
#define ADC_SQR1_LENGTH_MASK                (0xFU)
#define ADC_SEQUENCE_LENGTH_1_CONVERSION    (0U)
```

Then configures one conversion:

```c
ADC1_SQR1 &= ~(ADC_SQR1_LENGTH_MASK << ADC_SQR1_LENGTH_SHIFT);
ADC1_SQR1 |=  (ADC_SEQUENCE_LENGTH_1_CONVERSION << ADC_SQR1_LENGTH_SHIFT);
```

This means the regular ADC sequence contains one conversion.

## ADC Channel Selection

`ADC1_SQR3` controls the first conversions in the regular sequence.

For one channel, only `SQ1` is needed.

`SQ1` means the first channel to be converted when ADC conversion starts.

The code defines:

```c
#define ADC_SQR3_SQ1_SHIFT (0U)
#define ADC_SQR3_SQ1_MASK  (0x1FU)
#define ADC_CHANNEL_0      (0U)
```

Then selects ADC channel 0 as the first conversion:

```c
ADC1_SQR3 &= ~(ADC_SQR3_SQ1_MASK << ADC_SQR3_SQ1_SHIFT);
ADC1_SQR3 |=  (ADC_CHANNEL_0 << ADC_SQR3_SQ1_SHIFT);
```

This does not start conversion. It only tells the ADC which channel to read first.

The actual conversion is started later using `SWSTART` in `ADC1_CR2`.

## ADC Sample Time

`ADC1_SMPR2` controls the sample time for ADC channels 0 to 9.

Because PA0 maps to ADC channel 0, the firmware configures `SMP0[2:0]`, which is bits `2:0` in `ADC1_SMPR2`.

The code defines:

```c
#define ADC_SMPR2_SMP0_SHIFT    (0U)
#define ADC_SMPR2_SAMPLE_MASK   (0x7U)
#define ADC_SAMPLE_84_CYCLES    (0x4U)
```

Then sets the sample time:

```c
ADC1_SMPR2 &= ~(ADC_SMPR2_SAMPLE_MASK << ADC_SMPR2_SMP0_SHIFT);
ADC1_SMPR2 |=  (ADC_SAMPLE_84_CYCLES << ADC_SMPR2_SMP0_SHIFT);
```

`0x7U` is binary `111`, so it clears the 3-bit sample-time field.

`0x4U` is binary `100`, which selects 84 ADC clock cycles.

This means ADC channel 0 is sampled for 84 ADC clock cycles before conversion.

A longer sample time is useful for sensors such as an LDR voltage divider because it gives the ADC input more time to settle.

## ADC Control and Status Bits

| Macro | Purpose |
|---|---|
| `ADC_CR2_ADON` | Enables ADC1. |
| `ADC_CR2_SWSTART` | Starts ADC conversion by software. |
| `ADC_SR_EOC` | End-of-conversion flag. Set when conversion is complete. |

The ADC is enabled using:

```c
ADC1_CR2 |= ADC_CR2_ADON;
```

A conversion is started using:

```c
ADC1_CR2 |= ADC_CR2_SWSTART;
```

The firmware waits until conversion is complete:

```c
while (!(ADC1_SR & ADC_SR_EOC)) {}
```

Then reads the converted value:

```c
return ADC1_DR;
```

## ADC Initialisation Sequence

The ADC setup follows this order:

```text
1. Enable GPIOA clock.
2. Configure PA0 as analog mode.
3. Enable ADC1 clock through RCC_APB2ENR.
4. Set ADC regular sequence length to one conversion using ADC1_SQR1.
5. Select ADC channel 0 as the first conversion using ADC1_SQR3.
6. Set the sample time for channel 0 using ADC1_SMPR2.
7. Enable ADC1 using ADC1_CR2.
```

## ADC Read Sequence

Each ADC read follows this order:

```text
1. Start conversion using ADC1_CR2 SWSTART.
2. Wait for the EOC flag in ADC1_SR.
3. Read the converted value from ADC1_DR.
```

## Key ADC Code

```c
void adc1_pa0_init(void)
{
    /* Enable clock for PA0 */
    RCC_AHB1ENR |= GPIOAEN;

    /* Configure PA0 as analog */
    GPIOA_MODER |= PA0_MODE_BIT0;
    GPIOA_MODER |= PA0_MODE_BIT1;

    /* Enable ADC peripheral clock */
    RCC_APB2ENR |= ADC1EN;

    /* Set sequence length to 1 */
    ADC1_SQR1 &= ~(ADC_SQR1_LENGTH_MASK << ADC_SQR1_LENGTH_SHIFT);
    ADC1_SQR1 |=  (ADC_SEQUENCE_LENGTH_1_CONVERSION << ADC_SQR1_LENGTH_SHIFT);

    /* Select ADC channel 0 as the first regular conversion */
    ADC1_SQR3 &= ~(ADC_SQR3_SQ1_MASK << ADC_SQR3_SQ1_SHIFT);
    ADC1_SQR3 |=  (ADC_CHANNEL_0 << ADC_SQR3_SQ1_SHIFT);

    /* Set sample time */
    ADC1_SMPR2 &= ~(ADC_SMPR2_SAMPLE_MASK << ADC_SMPR2_SMP0_SHIFT);
    ADC1_SMPR2 |=  (ADC_SAMPLE_84_CYCLES << ADC_SMPR2_SMP0_SHIFT);

    /* Enable ADC1 */
    ADC1_CR2 |= ADC_CR2_ADON;
}

uint32_t adc1_read(void)
{
    /* Start ADC conversion */
    ADC1_CR2 |= ADC_CR2_SWSTART;

    /* Wait for conversion to complete */
    while (!(ADC1_SR & ADC_SR_EOC)) {}

    /* Read and return ADC result */
    return ADC1_DR;
}
```

## Integration with Security Light PWM

The ADC result is used to control the security light brightness.

```c
#define NIGHT_THRESHOLD 1200
```

In the main loop:

```c
light_value = adc1_read();

if (light_value < NIGHT_THRESHOLD)
{
    TIM3_CCR1 = 75;  // Security light on - 75% brightness
}
else
{
    TIM3_CCR1 = 0;   // Security light off
}
```

If the ADC value is below the threshold, the system treats the environment as dark and increases the PWM duty cycle.

If the ADC value is above the threshold, the security light is switched off by setting the PWM duty cycle to 0.

## SUMMARY

For the light detector, I used ADC1 Channel 0 because the sensor is connected to PA0, which maps to ADC1_IN0. I first configure PA0 as analog mode because the ADC needs to read the raw analog voltage on the pin. Then I enable the ADC1 peripheral clock through `RCC_APB2ENR`, since ADC1 is on the APB2 bus.

After enabling the ADC, I configure the regular conversion sequence. Since I am only reading one analog input, I set the sequence length to one conversion in `ADC1_SQR1`. Then I select channel 0 as the first conversion in `ADC1_SQR3`. I also configure the sample time in `ADC1_SMPR2`; in this case I use 84 ADC clock cycles to give the sensor signal time to settle.

To read the sensor, I start conversion using the `SWSTART` bit in `ADC1_CR2`, wait for the `EOC` flag in `ADC1_SR`, and then read the result from `ADC1_DR`. The returned ADC value is compared with a night threshold. If the value is below the threshold, the TIM3 PWM duty cycle is increased to turn on the security light. Otherwise, the duty cycle is set to zero.
