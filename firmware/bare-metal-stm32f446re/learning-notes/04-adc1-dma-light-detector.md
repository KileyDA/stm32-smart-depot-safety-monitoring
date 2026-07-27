# DMA2 Stream 0 ADC1 Light Detector Implementation

This note explains the DMA addition used to update the light detector ADC value automatically in the bare-metal STM32F446RE firmware.

## Purpose

Previously, the ADC value was read manually by the CPU:

```c
ADC1_CR2 |= ADC_CR2_SWSTART;

while (!(ADC1_SR & ADC_SR_EOC)) {}

return ADC1_DR;
```

This works, but it means the CPU must start the ADC conversion, wait for the conversion to finish, and then read the ADC data register.

With DMA, the ADC result is copied automatically from the ADC data register into memory.

The goal is:

```text
ADC1 reads PA0 continuously.
DMA2 copies ADC1_DR into adc_dma_value automatically.
The CPU checks adc_dma_value without polling ADC1_SR every time.
```

## System Context

The light detector is connected to PA0:

```c
#define LIGHT_DETECTOR (1U << 0) // PA0
```

PA0 maps to:

```text
PA0 -> ADC1_IN0 -> ADC channel 0
```

ADC1 converts the analog voltage from PA0 into a digital value. DMA2 then transfers the ADC result from `ADC1_DR` into the global variable:

```c
volatile uint16_t adc_dma_value = 0;
```

The security light PWM duty cycle is updated based on this value.

## Why DMA Is Used

DMA stands for Direct Memory Access.

It allows a peripheral to transfer data directly to memory without the CPU manually moving the data each time.

For this project:

```text
Source      = ADC1 data register
Destination = adc_dma_value variable
```

This means the CPU can focus on decision logic while ADC and DMA handle the sensor update in the background.

## Why `adc_dma_value` Is Global and Volatile

The ADC value is stored in a global variable:

```c
volatile uint16_t adc_dma_value = 0;
```

It is global so it has a fixed memory address for the whole program runtime.

It is marked `volatile` because the variable is updated by DMA, not directly by normal CPU code. This tells the compiler:

```text
Do not cache this value.
Always read the latest value from memory.
```

Without `volatile`, the compiler could assume the value does not change unexpectedly and optimise reads incorrectly.

## DMA2 Base Address

DMA2 is on the AHB1 bus.

The code defines:

```c
#define DMA2_OFFSET (0x00006400UL)
#define DMA2_BASE   (AHB1_BASE + DMA2_OFFSET)
```

This gives the base address for DMA2 registers.

## Important DMA2 Registers

| Register | Purpose |
|---|---|
| `DMA2_LIFCR` | Clears interrupt/status flags for lower DMA streams, including Stream 0. |
| `DMA2_S0CR` | Stream 0 control register. Configures and enables the DMA stream. |
| `DMA2_S0NDTR` | Number of data items to transfer. |
| `DMA2_S0PAR` | Peripheral address register. Stores the source peripheral address. |
| `DMA2_S0M0AR` | Memory 0 address register. Stores the destination memory address. |

## Important DMA Control Bits

| Macro | Purpose |
|---|---|
| `DMA_SCR_EN` | Enables the DMA stream. |
| `DMA_SCR_CHSEL_MASK` | Clears the DMA channel selection field. |
| `DMA_SCR_DIR_MASK` | Clears transfer direction bits. |
| `DMA_SCR_PSIZE_MASK` | Clears peripheral data size bits. |
| `DMA_SCR_MSIZE_MASK` | Clears memory data size bits. |
| `DMA_SCR_CIRC` | Enables circular mode. |
| `DMA_SCR_PSIZE_HALFWORD` | Sets peripheral data size to 16 bits. |
| `DMA_SCR_MSIZE_HALFWORD` | Sets memory data size to 16 bits. |
| `DMA_LIFCR_CLEAR_STREAM0` | Clears pending flags for DMA Stream 0. |

## Important ADC DMA Bits

| Macro | Purpose |
|---|---|
| `ADC_CR2_DMA` | Enables ADC DMA requests. |
| `ADC_CR2_DDS` | Keeps DMA requests active after each conversion. |
| `ADC_CR2_CONT` | Enables continuous ADC conversion mode. |
| `ADC_CR2_SWSTART` | Starts ADC conversion by software. |

## DMA Initialisation Sequence

The DMA setup follows this sequence:

```text
1. Enable DMA2 clock.
2. Disable DMA2 Stream 0 before configuration.
3. Wait until the stream is fully disabled.
4. Clear pending DMA Stream 0 flags.
5. Set the peripheral address to ADC1_DR.
6. Set the memory address to adc_dma_value.
7. Set the number of data items to transfer.
8. Select DMA Channel 0 for ADC1.
9. Set transfer direction to peripheral-to-memory.
10. Set peripheral data size to half-word.
11. Set memory data size to half-word.
12. Enable circular mode.
13. Enable DMA mode in ADC1.
14. Keep ADC DMA requests active after each conversion.
15. Enable ADC continuous conversion mode.
16. Enable DMA2 Stream 0.
```

## Key DMA Code

```c
void adc1_dma_init(void)
{
    /* 1. Enable DMA2 clock */
    RCC_AHB1ENR |= DMA2EN;

    /* 2. Disable DMA2 Stream 0 before configuring it */
    DMA2_S0CR &= ~DMA_SCR_EN;

    /* 3. Wait until DMA stream is fully disabled */
    while (DMA2_S0CR & DMA_SCR_EN) {}

    /* 4. Clear any pending flags for DMA2 Stream 0 */
    DMA2_LIFCR |= DMA_LIFCR_CLEAR_STREAM0;

    /* 5. Set peripheral address: ADC1 data register */
    DMA2_S0PAR = (uint32_t)&ADC1_DR;

    /* 6. Set memory address: variable where ADC result will be stored */
    DMA2_S0M0AR = (uint32_t)&adc_dma_value;

    /* 7. Set the number of data items to transfer */
    DMA2_S0NDTR = 1;

    /* 8. Select DMA Channel 0 for ADC1 */
    DMA2_S0CR &= ~DMA_SCR_CHSEL_MASK;

    /* 9. Set direction: peripheral-to-memory */
    DMA2_S0CR &= ~DMA_SCR_DIR_MASK;

    /* 10. Set peripheral data size to half-word */
    DMA2_S0CR &= ~DMA_SCR_PSIZE_MASK;
    DMA2_S0CR |= DMA_SCR_PSIZE_HALFWORD;

    /* 11. Set memory data size to half-word */
    DMA2_S0CR &= ~DMA_SCR_MSIZE_MASK;
    DMA2_S0CR |= DMA_SCR_MSIZE_HALFWORD;

    /* 12. Enable circular mode so that the value keeps updating */
    DMA2_S0CR |= DMA_SCR_CIRC;

    /* 13. Enable DMA mode in ADC1 */
    ADC1_CR2 |= ADC_CR2_DMA;

    /* 14. Keep DMA requests active after each conversion */
    ADC1_CR2 |= ADC_CR2_DDS;

    /* 15. Enable continuous conversion mode */
    ADC1_CR2 |= ADC_CR2_CONT;

    /* 16. Enable DMA2 Stream 0 */
    DMA2_S0CR |= DMA_SCR_EN;
}
```

## Why `DMA2_S0PAR = (uint32_t)&ADC1_DR`

```c
DMA2_S0PAR = (uint32_t)&ADC1_DR;
```

`DMA2_S0PAR` is the DMA peripheral address register.

DMA needs the address of the ADC data register, not the value currently stored inside it.

`ADC1_DR` represents the actual ADC data register. `&ADC1_DR` gives the memory address of that register.

The cast to `uint32_t` is used because the DMA address register stores the address as a 32-bit value.

This tells DMA:

```text
Read data from the ADC1 data register address.
```

This would be wrong:

```c
DMA2_S0PAR = ADC1_DR;
```

because that would store the current ADC reading as if it were an address.

## Why `DMA2_S0M0AR = (uint32_t)&adc_dma_value`

```c
DMA2_S0M0AR = (uint32_t)&adc_dma_value;
```

`DMA2_S0M0AR` is the DMA memory address register.

It tells DMA where to store the transferred ADC value.

`&adc_dma_value` gives the memory address of the global ADC variable.

This tells DMA:

```text
Store the ADC result in adc_dma_value.
```

## Why `DMA2_S0NDTR = 1`

```c
DMA2_S0NDTR = 1;
```

`NDTR` means Number of Data To Transfer.

The firmware currently reads one ADC channel and stores the latest reading in one variable.

Therefore, only one data item needs to be transferred:

```text
ADC1_DR -> adc_dma_value
```

Because circular mode is enabled, DMA repeatedly reloads this transfer count and keeps updating the same variable.

If a buffer was used instead, for example:

```c
volatile uint16_t adc_buffer[16];
```

then `NDTR` would be set to `16`, and memory increment would be enabled.

## Why Circular Mode Is Enabled

```c
DMA2_S0CR |= DMA_SCR_CIRC;
```

Circular mode means that when the transfer finishes, DMA automatically starts the transfer cycle again.

Since `NDTR = 1`, DMA transfers one ADC result, reloads, and waits for the next ADC result.

This allows:

```text
adc_dma_value
```

to always contain the latest ADC reading.

Without circular mode, DMA would transfer once and then stop.

## Why ADC DMA Mode Is Enabled

```c
ADC1_CR2 |= ADC_CR2_DMA;
```

This allows ADC1 to generate DMA requests when a conversion is complete.

Without this bit, ADC1 may still convert values, but DMA will not automatically transfer the result.

## Why DDS Is Enabled

```c
ADC1_CR2 |= ADC_CR2_DDS;
```

`DDS` keeps DMA requests active after each conversion.

This is useful for continuous ADC + DMA operation.

It allows ADC1 to keep requesting DMA transfers after every completed conversion rather than stopping after the first transfer.

## Why Continuous Conversion Mode Is Enabled

```c
ADC1_CR2 |= ADC_CR2_CONT;
```

Continuous conversion mode tells ADC1 to keep converting after the first software start.

Without continuous mode, the CPU would need to repeatedly start each conversion manually.

With continuous mode:

```text
Start once using SWSTART.
ADC keeps converting.
DMA keeps transferring.
adc_dma_value keeps updating.
```

## Why DMA Stream Is Enabled Last

```c
DMA2_S0CR |= DMA_SCR_EN;
```

The DMA stream is enabled last because all DMA settings should be configured before the stream starts.

The stream needs to know:

```text
which channel to use
where to read from
where to write to
how many data items to transfer
what data size to use
whether circular mode is enabled
```

After configuration is complete, setting `EN` makes DMA2 Stream 0 active and ready to respond to ADC1 DMA requests.

## Updated Main Flow

The main program initialises the peripherals in this order:

```c
security_light_pwm_init();
warning_led_init();
adc1_pa0_init();
adc1_dma_init();
adc1_start_conversion();
```

The ADC conversion is started once:

```c
ADC1_CR2 |= ADC_CR2_SWSTART;
```

After that, ADC1 and DMA2 update `adc_dma_value` in the background.

The main loop checks the latest value:

```c
if (adc_dma_value < NIGHT_THRESHOLD)
{
    TIM3_CCR1 = 75;  // Security light on - 75% brightness
}
else
{
    TIM3_CCR1 = 0;   // Security light off
}
```

## Key Difference from Polling

Before DMA:

```text
CPU starts ADC conversion.
CPU waits for EOC flag.
CPU reads ADC1_DR.
CPU uses the value.
```

With DMA:

```text
CPU starts ADC conversion once.
ADC converts continuously.
DMA transfers ADC1_DR into adc_dma_value.
CPU reads adc_dma_value when needed.
```

This reduces CPU involvement and makes the system cleaner for future expansion.

## SUMMARY

I added DMA so the ADC result can be moved automatically from the ADC data register into memory. The light detector is connected to PA0, which maps to ADC1 Channel 0. ADC1 converts that analog voltage, and DMA2 Stream 0 transfers the result from `ADC1_DR` into the global volatile variable `adc_dma_value`.

I configure the DMA peripheral address register with the address of `ADC1_DR`, and the DMA memory address register with the address of `adc_dma_value`. Since I only want the latest single ADC value, I set the number of data items to one. I use half-word data size because the ADC result is 12-bit and fits inside 16 bits.

Circular mode is enabled so DMA keeps updating the same variable repeatedly. In ADC1, I enable DMA mode, DDS, and continuous conversion mode. Then I enable the DMA stream and start ADC conversion once. After that, ADC and DMA work in the background, and the main loop simply compares `adc_dma_value` with the night threshold to decide whether to turn on the security light PWM.
