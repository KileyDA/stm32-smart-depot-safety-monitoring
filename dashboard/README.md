# Smart Depot Safety Monitoring — Dashboard

## Overview

This directory contains the Python communication and dashboard components of the **STM32 Smart Depot Safety Monitoring System**.

The objective is to develop a web-based monitoring interface that displays real-time sensor readings, security light status and alert activity. Future development will also allow users to acknowledge alerts and modify system parameters through the website.

**Current stage:** STM32-to-Python serial communication and telemetry parsing completed. Web dashboard development is next.

## 1. Current Architecture

```text
STM32F446RE
    |
    | USART2 (PA2 TX / PA3 RX)
    |
ST-LINK USB Virtual COM Port
    |
    | COM9 — 115200 baud
    |
Python Serial Reader (PySerial)
    |
    |
Telemetry Parser (protocol.py)
    |
    v
Structured Python Data
```

The STM32 handles sensor acquisition and alert control independently. Python currently receives and processes monitoring data without modifying the embedded system's behaviour.

## 2. Implemented Features

### STM32 Serial Communication

- Configured USART2 for serial communication through the onboard ST-LINK USB interface.
- Implemented character, text and integer transmission functions in C.
- Added SysTick-based status reporting at 500 ms intervals.
- Created structured telemetry messages containing sensor readings and system states.

### Python Serial Reader

- Established communication with the STM32 using PySerial.
- Receives newline-terminated messages through the virtual COM port.
- Decodes incoming serial data into readable strings.
- Identifies STM32 startup messages and processes telemetry.

### Telemetry Parsing

The `protocol.py` module:

- Splits comma-separated telemetry messages.
- Validates message format and numerical ranges.
- Converts numerical status flags into Python boolean values.
- Returns structured dictionaries for future web application integration.

## 3. Telemetry Message Format

The STM32 sends messages using the format:

```text
STATE,adc,motion,alert,acknowledged,brightness
```

Example:

```text
STATE,1104,0,0,0,50
```

| Field | Description |
|---|---|
| `STATE` | Message identifier |
| `adc` | Light sensor ADC reading (0–4095) |
| `motion` | PIR motion state (0 or 1) |
| `alert` | Latched alert state (0 or 1) |
| `acknowledged` | Alert acknowledgement state (0 or 1) |
| `brightness` | Security light PWM duty cycle (0–100%) |

After parsing, Python produces:

```python
{
    "adc_value": 1104,
    "motion_detected": False,
    "alert_active": False,
    "alert_acknowledged": False,
    "brightness": 50
}
```

## 4. Project Files

```text
dashboard/
├── .gitignore
├── README.md
├── protocol.py
├── serial_reader.py
└── requirement.txt
```

**`serial_reader.py`** — Opens the serial connection, receives STM32 messages and displays parsed telemetry.

**`protocol.py`** — Validates and converts telemetry into structured Python dictionaries.

**`requirement.txt`** — Lists the Python package dependencies.

**`.gitignore`** — Excludes local virtual environments, Python caches and other generated files from version control.

## 5. Running the Serial Monitor

### Prerequisites

- Python 3
- STM32 NUCLEO-F446RE with updated firmware
- USB connection to the onboard ST-LINK interface
- PySerial

### Setup

From the `dashboard` directory, create and activate a Python virtual environment:

```powershell
python -m venv .venv
.\.venv\Scripts\Activate.ps1
```

Install dependencies:

```powershell
python -m pip install -r requirement.txt
```

Identify the STM32 virtual COM port:

```powershell
python -m serial.tools.list_ports -v
```

Update the `PORT` variable in `serial_reader.py` if necessary.

Run the serial monitor:

```powershell
python serial_reader.py
```

The current development configuration uses COM9 at 115200 baud. The COM port may differ between computers.

## 6. Testing and Verification

The following functionality has been tested successfully:

- STM32 startup message (`STM32 READY`) received by Python.
- Continuous telemetry received from the STM32.
- ADC light readings change with environmental lighting.
- PIR motion states are transmitted.
- Latched alerts remain active after the PIR output returns LOW.
- PWM brightness and alert states are correctly represented in Python dictionaries.

These tests establish the communication foundation for the web dashboard.

## 7. Next Development Milestones

- **FastAPI backend:** Integrate serial reception into a Python web server.
- **Live dashboard:** Display ADC readings, PIR activity, brightness and alert state in a browser.
- **Real-time updates:** Add WebSocket communication.
- **Bidirectional control:** Send commands from the website to the STM32 to adjust monitoring parameters and acknowledge alerts.
- **Event history:** Store alert events and monitoring data using SQLite.
- **Mobile access:** Make the dashboard accessible on a phone through the local network.
- **ESP32 integration:** Introduce wireless communication in a later development phase.

## 8. Design Principle

The STM32 remains responsible for physical sensor monitoring, security light control and alert-state management.

The Python application provides a separate communication, visualization and future control layer. This separation allows the embedded system to continue operating even when the dashboard is disconnected.
