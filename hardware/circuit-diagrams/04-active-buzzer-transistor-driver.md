# 04 - Active Buzzer Transistor Driver Circuit

## Purpose

This circuit adds an audible alert output to the Smart Depot Safety and Access Monitoring System.

The active buzzer represents the alarm sound that will be triggered during an alert condition. For example, when the system is in **Night Monitoring Mode** and motion is detected by the PIR sensor, the buzzer can be turned on to warn that activity has been detected in the monitored depot area.

## Circuit Concept

The active buzzer is a 5V electromagnetic buzzer. It is rated for 3.5V–5.5V and can draw up to approximately 25mA.

Because the buzzer requires more current than should be supplied directly from an STM32 GPIO pin, it is controlled using a **PN2222 NPN transistor driver circuit**.

The STM32 does not power the buzzer directly. Instead, it sends a small control signal to the base of the PN2222 transistor through a resistor. The PN2222 then acts as an electronic switch, allowing the buzzer current to flow from the 5V supply.

```text
STM32 GPIO signal → base resistor → PN2222 base
5V supply → active buzzer → PN2222 transistor → GND
```

This protects the STM32 GPIO pin and allows the microcontroller to safely control a higher-current output device.

## Components Used

| Component             | Purpose                                                             |
| --------------------- | ------------------------------------------------------------------- |
| STM32 Nucleo F446RE   | Main microcontroller                                                |
| 5V active buzzer      | Produces audible alert sound                                        |
| PN2222 NPN transistor | Switches the buzzer current using a small STM32 GPIO control signal |
| 1kΩ resistor          | Limits current from the STM32 GPIO pin into the transistor base     |
| 1N4007 diode          | Provides flyback protection across the electromagnetic buzzer       |
| Breadboard            | Circuit prototyping                                                 |
| Jumper wires          | Electrical connections                                              |

## Active Buzzer Specification

| Parameter         | Value                         |
| ----------------- | ----------------------------- |
| Buzzer type       | 5V active buzzer              |
| Operating voltage | 3.5V–5.5V                     |
| Current           | < 25mA                        |
| Frequency         | 2300 ± 500Hz                  |
| Type              | Electromagnetic active buzzer |

An active buzzer was selected because it already contains an internal oscillator. This means the STM32 only needs to send a digital HIGH or LOW signal to turn the buzzer ON or OFF.

```text
GPIO HIGH → buzzer ON
GPIO LOW  → buzzer OFF
```

## Why a Transistor Driver Is Used

The STM32 GPIO pin outputs about 3.3V and should only be used for small control signals. The buzzer is a 5V device and can draw up to approximately 25mA. If the buzzer were connected directly to the STM32 GPIO pin, the pin would have to supply the buzzer current itself, which could overload or damage the microcontroller pin.

With the transistor driver, the 5V supply powers the buzzer, while the STM32 only controls the transistor.

```text
STM32 GPIO pin → small control current only
5V supply → buzzer current
PN2222 → switches the buzzer current
```

## Why Low-Side Switching Is Used

The PN2222 is used as a **low-side switch**. This means the buzzer is connected to 5V on one side, while the transistor is placed between the buzzer negative terminal and ground.

```text
5V → active buzzer → PN2222 collector
PN2222 emitter → GND
```

When the STM32 GPIO pin goes HIGH, current flows into the base of the PN2222. This turns the transistor ON and creates a path from the buzzer to ground.

```text
5V → active buzzer → PN2222 → GND
```

The buzzer sounds.

When the STM32 GPIO pin goes LOW, the transistor turns OFF. The path to ground is broken, so the buzzer stops.

Low-side switching is used because it works well with an NPN transistor controlled by a 3.3V microcontroller signal.

## Why an NPN Transistor Is Suitable

The PN2222 is an NPN transistor. This makes it suitable for low-side switching because it turns ON when a small positive current is applied to its base.

```text
STM32 GPIO HIGH → small base current flows → PN2222 turns ON → buzzer sounds
STM32 GPIO LOW  → no base current flows    → PN2222 turns OFF → buzzer stops
```

This allows the STM32 to control the ground path of the buzzer while the buzzer itself is powered from the 5V supply.

## Why the PN2222 Transistor Was Chosen

The PN2222 was chosen because its datasheet values are suitable for switching the active buzzer.

| Selection Criterion         |                           PN2222  Value | Relevance to This Circuit                                                                                                                                            |
| ------------------------------- | --------------------------------: | -------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Transistor type                 |                               NPN | Suitable for low-side switching because it can be placed between the buzzer and ground, allowing a 3.3V STM32 GPIO signal to switch the 5V buzzer current ON and OFF |
| Collector current, IC           |                             600mA | Much higher than the buzzer current of <25mA                                                                                                                         |
| Collector-emitter voltage, VCEO |                               30V | Much higher than the 5V buzzer supply                                                                                                                                |
| Collector power dissipation, PC |                             625mW | Suitable for this low-current switching circuit                                                                                                                      |
| Saturation test condition       |             IC = 500mA, IB = 50mA | Gives a forced switching gain of 10 for base resistor calculation                                                                                                    |
| Pinout                          | 1: Emitter, 2: Base, 3: Collector | Needed to wire the transistor correctly                                                                                                                              |

The most important comparison is the current rating:

```text
Buzzer current: <25mA
PN2222 collector current rating: 600mA
```

The buzzer current is far below the PN2222 collector current rating, so the transistor can safely switch the buzzer.

The voltage rating is also suitable:

```text
Buzzer supply voltage: 5V
PN2222 collector-emitter voltage rating: 30V
```

Since the circuit uses only 5V, the PN2222 voltage rating is safely above the required value.

The power dissipation rating matters because any voltage drop across the transistor while current is flowing will produce heat. In this circuit, the buzzer current is small, and the PN2222 is being driven as a switch, so the transistor is expected to dissipate only a small amount of power compared with its 625mW rating.

## Why a Base Resistor Is Used

The STM32 GPIO pin is connected to the PN2222 base through a resistor to limit the current flowing into the transistor.

In an NPN transistor, the base-emitter junction behaves similarly to a diode. When the STM32 GPIO pin goes HIGH, the base becomes more positive than the emitter. This allows current to flow from the GPIO pin, through the base resistor, into the transistor base, and then to ground through the emitter.

```text
STM32 GPIO → base resistor → PN2222 base → PN2222 emitter → GND
```

Without the base resistor, this base-emitter path could draw too much current from the STM32 GPIO pin. This could overload or damage the microcontroller pin. The base resistor prevents this by limiting the base current to a safe value while still allowing enough current to turn the transistor ON.

A 1kΩ resistor was selected as a practical base resistor for this circuit.

The active buzzer can draw up to approximately 25mA. To switch the buzzer reliably, the PN2222 should be driven fully ON, also called saturation.

From the PN2222 datasheet, the transistor saturation test uses:

```text
Collector current, IC = 500mA
Base current, IB = 50mA
```

The ratio between collector current and base current is therefore:

```text
Forced transistor gain = IC / IB
Forced transistor gain = 500mA / 50mA
Forced transistor gain = 10
```

This means that, for reliable switching, the circuit can be designed using a conservative forced gain of about 10. In simple terms, for every 10mA of load current, about 1mA of base current is provided.

For this buzzer circuit:

```text
Buzzer current ≈ 25mA
Forced transistor gain for switching ≈ 10

Required base current ≈ 25mA / 10
Required base current ≈ 2.5mA
```

The STM32 GPIO output is about 3.3V. When the PN2222 transistor is ON, the base-emitter junction behaves similarly to a forward-biased silicon diode. A common practical estimate for the base-emitter voltage of a silicon transistor is approximately 0.7V.

Source for the 0.7V base-emitter estimate:

```text
https://www.electronics-tutorials.ws/transistor/tran_2.html
```

This means the voltage across the base resistor is approximately:

```text
Voltage across base resistor ≈ GPIO voltage - base-emitter voltage
Voltage across base resistor ≈ 3.3V - 0.7V
Voltage across base resistor ≈ 2.6V
```

Using Ohm’s law:

```text
Base resistor ≈ voltage across resistor / required base current
Base resistor ≈ 2.6V / 2.5mA
Base resistor ≈ 2.6V / 0.0025A
Base resistor ≈ 1040Ω
```

A standard 1kΩ resistor is therefore a suitable choice. It provides enough base current(2.6mA) to switch the PN2222 ON while keeping the STM32 GPIO current within a safe range.

## Why a Flyback Diode Is Used

The active buzzer is electromagnetic, so it contains a coil. When current flows through the buzzer, energy is stored in the coil as a magnetic field.

```text
5V → active buzzer coil → PN2222 transistor → GND
```

When the PN2222 transistor switches OFF, this current path is suddenly broken. However, the coil resists the sudden drop in current and tries to keep the current flowing. To do this, it can generate a reverse voltage that may become much higher than the normal 5V supply voltage. This sudden voltage is called a **flyback voltage** or **back EMF**.

If there is no safe path for this current, the voltage spike could appear at the PN2222 collector and may damage the transistor or disturb the STM32 circuit.

A 1N4007 flyback diode is placed across the buzzer to provide a safe path for this stored energy when the transistor switches OFF. 

```text
1N4007 cathode / striped side   → buzzer positive / 5V side
1N4007 anode / non-striped side → buzzer negative / PN2222 collector side
```
During normal buzzer operation, the diode is reverse-biased and does not conduct. When the buzzer coil generates a reverse voltage spike, the diode becomes forward-biased and allows the coil current to circulate safely until the stored magnetic energy dies away.

This protects the PN2222 transistor and the STM32 circuit from the voltage spike.


## Why the 1N4007 Diode Was Chosen

The 1N4007 was chosen because it is a general-purpose rectifier diode suitable for flyback protection in this low-current buzzer circuit.

| 1N4007 Datasheet Value                |                       Value | Relevance to This Circuit                        |
| ------------------------------------- | --------------------------: | ------------------------------------------------ |
| Diode type                            |   General-purpose rectifier | Suitable for flyback protection                  |
| Peak repetitive reverse voltage, VRRM |                       1000V | Much higher than the 5V buzzer supply            |
| Average forward current, IF(AV)       |                          1A | Much higher than the buzzer current of <25mA     |
| Surge current capability, IFSM        |                         30A | Can tolerate short transient current spikes      |
| Cathode marking                       | Colour band denotes cathode | Helps connect the diode in the correct direction |

The 1N4007 ratings are much higher than the buzzer circuit requirements. This makes it suitable for protecting the circuit from the voltage spike produced when the buzzer is switched OFF.

## PN2222 Pinout Note

The PN2222 datasheet shows the TO-92 pinout as:

```text
1 - Emitter
2 - Base
3 - Collector
```

Before wiring, the transistor should be placed according to the datasheet pinout and the physical orientation of the package.

In this circuit:

```text
Emitter   → GND
Base      → STM32 GPIO pin through 1kΩ resistor
Collector → buzzer negative side
```

## Pin Connections

Update the STM32 GPIO pin below based on the final pin selected in CubeMX.

| Circuit Node                    | Connection                         |
| ------------------------------- | ---------------------------------- |
| STM32 GPIO output               | 1kΩ base resistor                  |
| 1kΩ base resistor               | PN2222 base                        |
| PN2222 emitter                  | GND                                |
| PN2222 collector                | Buzzer negative                    |
| Buzzer positive                 | 5V                                 |
| Buzzer negative                 | PN2222 collector                   |
| 1N4007 cathode / striped side   | Buzzer positive / 5V               |
| 1N4007 anode / non-striped side | Buzzer negative / PN2222 collector |
| STM32 GND                       | Common GND                         |

## Circuit Connection

```text
STM32 GPIO pin → 1kΩ resistor → PN2222 base

5V → active buzzer positive
active buzzer negative → PN2222 collector
PN2222 emitter → GND

1N4007 diode across buzzer:
striped side → buzzer positive / 5V
non-striped side → buzzer negative / PN2222 collector
```

The STM32 GND and buzzer supply GND must be connected together. This gives the STM32 control signal and the transistor driver circuit the same voltage reference.

## CubeMX Configuration

The selected STM32 pin should be configured as a normal GPIO output.

| Peripheral / Pin     | Configuration               |
| -------------------- | --------------------------- |
| Selected GPIO pin    | GPIO Output                 |
| User Label           | ACTIVE_BUZZER               |
| GPIO Output Level    | Low                         |
| GPIO Mode            | Output Push Pull            |
| Pull-up / Pull-down  | No pull-up and no pull-down |
| Maximum Output Speed | Low                         |

The output is initially set to LOW so that the transistor remains OFF and the buzzer stays silent when the system starts.

## Firmware Test

The firmware test turns the buzzer ON and OFF using the selected GPIO output pin.

Example test code:

```c
HAL_GPIO_WritePin(ACTIVE_BUZZER_GPIO_Port, ACTIVE_BUZZER_Pin, GPIO_PIN_SET);
HAL_Delay(300);

HAL_GPIO_WritePin(ACTIVE_BUZZER_GPIO_Port, ACTIVE_BUZZER_Pin, GPIO_PIN_RESET);
HAL_Delay(700);
```

Expected behaviour:

```text
Buzzer ON briefly
Buzzer OFF
Repeats
```

## Test Result

The active buzzer transistor driver circuit will be tested by confirming that the STM32 can switch the buzzer ON and OFF using a GPIO output.

Observed behaviour:

```text
GPIO HIGH → PN2222 turns ON  → buzzer sounds
GPIO LOW  → PN2222 turns OFF → buzzer stops
```

This confirms that the STM32 can safely control an audible alert output through a transistor driver.

## Next Step

The next step is to combine the buzzer with the LDR, PIR, and LED logic.

Planned behaviour:

```text
Day Mode:
Security Light OFF
Warning LED OFF
Buzzer OFF

Night Monitoring Mode:
Security Light LOW brightness
Warning LED OFF
Buzzer OFF

Alert Mode:
Security Light HIGH brightness
Warning LED ON
Buzzer ON

Fault Mode:
Warning LED BLINKING
Buzzer pulsed or OFF depending on final design
```

This will allow the project to move from individual circuit testing into full system behaviour.
