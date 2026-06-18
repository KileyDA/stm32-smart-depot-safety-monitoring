# 01 - LDR Voltage Divider Circuit

## Purpose

This circuit is used to detect the light level around the monitored depot area. The LDR/photoresistor changes resistance depending on brightness, and the STM32 reads the resulting voltage through its ADC pin.

This is the first sensing stage of the Smart Depot Safety and Access Monitoring System. It will later be used to decide whether the system should operate in **Day Mode** or **Night Monitoring Mode**.

## Circuit Concept

The STM32 cannot directly measure resistance, so the LDR is connected with a fixed 10kΩ resistor to form a voltage divider. The voltage at the middle point of the divider changes as the light level changes.

![LDR Voltage Divider Circuit](https://github.com/KileyDA/stm32-smart-depot-safety-monitoring/blob/hardware/circuit-diagrams/images/circuit-diagrams/01-ldr-voltage-divider.png)


## Components Used

| Component           | Purpose                                  |
| ------------------- | ---------------------------------------- |
| STM32 Nucleo F446RE | Main microcontroller                     |
| LDR / Photoresistor | Detects light level                      |
| 10kΩ resistor       | Forms voltage divider with the LDR       |
| Breadboard          | Circuit prototyping                      |
| Jumper wires        | Connections between STM32 and breadboard |

## Pin Connections

| Circuit Node                                  | STM32 Connection    |
| --------------------------------------------- | ------------------- |
| Top of LDR                                    | 3.3V                |
| Middle junction between LDR and 10kΩ resistor | A0 / PA0 / ADC1_IN0 |
| Bottom of 10kΩ resistor                       | GND                 |

## Why a 10kΩ Resistor Is Used

The 10kΩ resistor is used as the lower resistor in the voltage divider. It connects the ADC signal point to ground through a known resistance, instead of shorting the ADC pin directly to ground. This allows the signal at A0 / PA0 to vary between 0V and 3.3V depending on the LDR resistance.

The LDR is connected to 3.3V, while the 10kΩ resistor is connected to ground. This means the ADC signal point is influenced by both sides of the circuit. The LDR side pulls the signal point toward 3.3V, while the 10kΩ resistor pulls it toward 0V. The voltage measured at A0 depends on the resistance balance between the LDR and the fixed resistor.

In bright light, the LDR resistance decreases, so the ADC signal point is pulled more strongly toward 3.3V. This causes the ADC value to increase. In darker conditions, the LDR resistance increases, so the signal point is pulled less strongly toward 3.3V and more toward ground through the 10kΩ resistor. This causes the ADC value to decrease.

Together, the LDR and the 10kΩ resistor convert changes in light level into a changing voltage that the STM32 ADC can measure. The 10kΩ value is a practical starting choice because it provides a useful ADC voltage range for detecting bright and dark conditions while keeping the current low.

Since this circuit is only used for sensing light and not powering anything, the ADC pin only needs to measure a voltage and does not require a large current. Keeping the current low:
- Protects the components: Too much current could heat or damage the LDR or waste power unnecessarily.
- Reduces power consumption: Since the system will be running for long periods of time
- Keeps the circuit efficient: ADC pin only needs a voltage reading,not a strong current signal
- Avoids unnecessary loading: it prevents excessive current flow from 3.3V to ground which would be wasteful.
  
## CubeMX Configuration

The project was configured in STM32CubeMX using the **NUCLEO-F446RE** board.

| Peripheral      | Configuration |
| --------------- | ------------- |
| ADC1            | Enabled       |
| PA0             | ADC1_IN0      |
| USART2          | Asynchronous  |
| Baud Rate       | 115200        |
| Toolchain / IDE | STM32CubeIDE  |

## Firmware Test

The STM32 reads the ADC value from PA0 and sends the result over USART2 to PuTTY through the ST-LINK Virtual COM Port.

Example serial output:

```text
LDR ADC Value: 3922 | Voltage: 3160 mV
LDR ADC Value: 2926 | Voltage: 2357 mV
```

## Serial Monitor Settings

PuTTY was configured using the following settings:

| Setting         | Value  |
| --------------- | ------ |
| Connection type | Serial |
| Serial line     | COM11  |
| Baud rate       | 115200 |
| Data bits       | 8      |
| Stop bits       | 1      |
| Parity          | None   |
| Flow control    | None   |

## Test Result

The LDR voltage divider was tested under three lighting conditions: bright light, normal room light, and covered/dark. The ADC readings were transmitted through USART2 and viewed in PuTTY.

The STM32 ADC was configured as a 12-bit ADC with a 3.3V reference voltage. This means the ADC output ranges from 0 to 4095, where 0 represents 0V and 4095 represents approximately 3.3V. The voltage values were calculated using the following formula:

Voltage_mV = (ADC_Value * 3300) / 4095;

| Lighting Condition | Approx. ADC Range | Approx. Voltage Range |
|---|---:|---:|
| Bright light | 3100 – 3816 | 2500 – 3075 mV |
| Normal room light | 1870 – 2160 | 1500 – 1740 mV |
| Covered / dark | 380 – 620 | 300 – 500 mV |

The results show that the ADC value increases when the LDR is exposed to brighter light and decreases when the LDR is covered, matching the expected behaviour of the voltage divider circuit.

In bright light, the LDR resistance decreases, pulling the ADC signal point closer to 3.3V and producing a higher ADC reading. In darker conditions, the LDR resistance increases, so the ADC signal point is pulled closer to ground through the 10kΩ resistor, producing a lower ADC reading.

This confirms that the LDR voltage divider is working correctly and that the STM32 can read light-level changes through ADC1 channel 0.

## Initial Threshold Selection

A starting night-mode threshold of 1200 ADC counts was selected because it lies between the measured normal room light range and the covered/dark range. The covered/dark readings were approximately 380–620, while the normal room light readings were approximately 1870–2160. Therefore, 1200 provides a clear separation point for the first firmware test.

If the ADC value falls below 1200, the system can classify the environment as dark and switch to Night Monitoring Mode. This threshold may be adjusted later after testing in different real-world lighting conditions.

```c
#define NIGHT_THRESHOLD 1200
```
