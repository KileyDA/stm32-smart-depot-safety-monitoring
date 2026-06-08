# STM32-Based Smart Depot Safety and Access Monitoring System

## Overview
This project is a smart embedded access monitoring system designed for vehicle depots, truck yards, workshops, loading bays, and restricted industrial areas.

Vehicle depots and truck yards often have parked vehicles, tools, equipment, fuel areas, and restricted zones that need to be monitored, especially at night. Poor lighting, blind spots, and delayed awareness of movement can create safety and security risks. This project explores how a low-cost embedded system can help monitor such areas by detecting light conditions, identifying motion activity, triggering local alerts, and preparing event data for a future web dashboard.

The system uses an STM32 F446RE microcontroller as the main controller. It reads light levels using an LDR, detects motion using a PIR sensor, controls warning outputs such as LEDs and a buzzer, and will later send status data to a dashboard.

## Why I Am Building This

I am building this project to strengthen my skills in embedded software engineering while working on a concept that is relevant to the automotive and industrial sectors.

Instead of treating this as just another microcontroller project, I wanted to frame it around a realistic use case: monitoring activity around a vehicle depot or restricted industrial area. This allows me to learn core embedded concepts while also thinking about how the system could be useful in a real environment.

This project connects to my interest in:

- Embedded software engineering
- Automotive and fleet technology
- Industrial monitoring systems
- IoT-based safety systems
- Sensor-driven automation
- Full-stack dashboard integration

## Problem Statement

Basic motion lights can detect movement and turn on a light, but they do not provide much context. They usually do not know whether the movement happened during the day or night, whether the alert was acknowledged, how often alerts happened, or whether the event should be logged for later review.

In depot or industrial environments, this context matters. A system that can detect conditions, respond locally, and later report events to a dashboard is more useful than a simple standalone alarm.

This project aims to create a small embedded monitoring node that can:

- Detect whether the environment is bright or dark
- Activate lighting only when needed
- Detect motion in a monitored area
- Trigger an alert during night-time movement
- Allow a user to acknowledge or reset the alert
- Prepare structured system data for future dashboard integration

  ## System Concept

The system is designed as a small monitoring node that could be placed near a depot gate, loading bay, workshop entrance, vehicle parking area, or restricted zone.

During the day, the system remains in normal monitoring mode and keeps the security light and alarm outputs off.

When the environment becomes dark, the system enters night monitoring mode and activates the security light. If motion is detected while it is dark, the system enters alert mode by switching on a warning LED and buzzer. A reset button allows the alert to be acknowledged.

Later, the STM32 will send system data through serial communication to a backend service, where the information can be displayed on a web dashboard.

## Key Features
- Light-level detection using an LDR/photoresistor
- Motion detection using an HC-SR501 PIR sensor
- Automatic day/night mode selection
- Security light control
- Warning LED and buzzer alert
- Manual reset/acknowledge button
- Timer-based alert behaviour
- State-machine-based embedded logic
- Fault handling for unexpected sensor conditions
- Serial data output for future dashboard integration

  ## Hardware Components
- STM32 Nucleo F446RE
- Photoresistor / LDR
- HC-SR501 PIR Motion Sensor
- LEDs
- Active Buzzer
- Push Button
- Resistors
- Breadboard
- Jumper Wires

## System Modes
**Day Mode**

The environment is bright, so the security light and alarm outputs remain off.

**Night Monitoring Mode**

The environment is dark, so the security light turns on and the system actively monitors for motion.

 **Alert Mode**

Motion is detected during night monitoring. The warning LED and buzzer turn on, and the system records the alert condition.

 **Acknowledged Mode**

The reset button is pressed. The alert is silenced, and the system returns to the correct monitoring mode depending on the current light condition.

**Fault Mode**

The system detects an invalid or unexpected sensor condition and enters a safe fault-handling state instead of continuing normal operation blindly.

## Embedded Concepts Covered

This project is being developed step by step so that each embedded concept is understood properly before moving to the next stage.

The main concepts covered include:

- ADC input processing
- GPIO input and output
- Digital sensor interfacing
- PWM brightness control
- Timer-based control
- Button debouncing
- Hysteresis for stable switching
- Finite state machine design
- Fault handling
- UART/serial communication
- Embedded-to-dashboard integration

## Development Roadmap
**Phase 1: System Planning**
Defined the project concept
Reframed the project from a basic security light system into a smart depot safety and access monitoring system
Selected the hardware components
Planned the operating modes
Designed the embedded system architecture

**Phase 2: Hardware Development**
Build the LDR voltage divider circuit
Test ADC light-level readings
Connect the security LED
Connect the PIR motion sensor
Connect the warning LED and buzzer
Add the reset/acknowledge button

**Phase 3: Embedded Firmware**
Implement day/night detection
Add hysteresis to prevent unstable switching
Implement motion-triggered alert logic
Add timer-based alert control
Implement reset button handling
Refactor the logic into a finite state machine
Add fault mode

**Phase 4: Serial Communication**
Send system status over UART/USB serial
Define structured event messages
Prepare sample serial logs for dashboard testing

Example future serial message:

STATE=ALERT,LIGHT=320,MOTION=1,ALARM=1

**Phase 5: Dashboard Integration**
Build a Flask backend to read serial data
Store alert events
Display live system status
Show event history and alert count
Add dashboard screenshots and demo notes

## Expected Outcome
By the end of the project, the system should demonstrate a complete embedded monitoring workflow:

- Sense the environment
- Classify the system condition
- Make a real-time decision
- Trigger physical outputs
- Send or log system status
- Prepare the data for dashboard monitoring

The goal is to create a project that shows both technical embedded skills and real-world thinking around safety, monitoring, and system integration.
  
