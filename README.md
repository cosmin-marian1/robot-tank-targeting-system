# Robot Tank with Targeting System

## Overview

This project presents a compact tracked robot designed to analyze its surroundings and detect nearby obstacles. One of its key features is its modern control method: instead of using a traditional radio remote, the robot is operated through a web interface accessible directly from a mobile phone.

The system also includes a prototype **Targeting System**, focused on **active safety**. Its main role is to block the firing action whenever an object is detected at an unsafe distance. Although the implementation involved several hardware limitations and engineering challenges, the final result successfully met the initial project goals.
![WhatsApp Image 2026-03-31 at 18 14 35](https://github.com/user-attachments/assets/9f47d6c6-7700-4e51-b76b-13231dcbf5d7)

## Project Objectives

- Build a compact and agile tracked robot
- Control the robot through a wireless web interface
- Detect obstacles using an ultrasonic sensor
- Rotate the targeting system with a servo motor
- Implement a laser firing mechanism with a safety lock
- Gain practical experience in embedded systems, robotics, and control logic

## System Architecture

The robot is built around an **ESP8266** microcontroller, which creates a Wi-Fi access point and hosts the web control interface. Through this interface, the user can:

- drive the robot in real time,
- rotate the turret,
- monitor obstacle distance,
- activate the laser system when safety conditions are met.

### Main subsystems

- **ESP8266** – central controller and Wi-Fi access point
- **L298N motor driver** – controls the DC motors
- **DC motors** – provide tracked movement
- **SG90 servo motor** – rotates the ultrasonic sensor / turret assembly
- **Ultrasonic sensor** – measures the distance to obstacles
- **Laser module** – simulates the firing system
- **Battery supply** – powers the full system

## Hardware Components

The following components were used in this project:

- Lightweight wooden chassis base
- Two 3.7V Li-Ion batteries
- Four DC motors
- SG90 servo motor
- Ultrasonic sensor
- Laser module
- ESP8266 development board
- L298N motor driver
- Jumper wires and mounting accessories
- 100µF electrolytic capacitor for power stabilization

## Assembly Process

The mechanical and electrical assembly required careful planning because the robot chassis offered limited mounting space.

### Main assembly steps

1. Prepare the wooden chassis and mount the tracked drive system.
2. Install the DC motors and connect them to the L298N motor driver.
3. Mount the ESP8266 board in a protected and accessible position.
4. Attach the servo motor and place the ultrasonic sensor on top for better field of view.
5. Install the laser module on the turret assembly.
6. Route and organize the wiring to avoid interference with the tracks and moving parts.
7. Connect the battery pack and verify the stability of the power supply.

A layered arrangement of components was used to improve cable management and reduce the risk of mechanical obstruction or accidental short circuits.

## Software

### Development tools

- Arduino IDE
- ESP8266WiFi library
- ESP8266WebServer library
- Servo library

### Software features

The ESP8266 firmware handles:

- Wi-Fi access point creation
- hosting the web-based control interface
- motor control for movement
- turret rotation
- ultrasonic distance measurement
- laser activation with safety validation
- emergency stop functionality

### Control interface

The web interface includes:

- a virtual joystick for drive control,
- a virtual joystick for turret rotation,
- a radar-style distance display,
- a **SHOOT** button,
- an emergency **STOP** button.

The interface is optimized for mobile devices and allows intuitive real-time control.

### Safety logic

A major feature of the project is the **laser safety lock**:

- if an obstacle is detected closer than **20 cm**, firing is blocked;
- the interface displays a warning message;
- the shoot button is disabled;
- the server also performs a second validation before activating the laser.

This ensures that the firing action is not allowed when the target is too close.

## Main Code Structure

The program is organized into several functional sections:

- **Pin definitions** for motors, servo, laser, and ultrasonic sensor
- **Wi-Fi configuration**
- **Web page definition** using embedded HTML/CSS/JavaScript
- **Motor control functions**
- **Turret control functions**
- **Distance measurement logic**
- **Laser firing logic**
- **Server route handlers**
- **Main loop** for non-blocking event handling

## Challenges and Problems Encountered

### 1. Limited physical space

**Problem:**  
The robot chassis had very limited space for all the required components, including the motor driver, ESP8266 board, batteries, and sensors. Poor cable placement could interfere with the tracks or create accidental short circuits.

**Solution:**  
A layered architecture was adopted. Power wires and signal wires were grouped separately, and the sensor-servo assembly was placed on the upper section to provide a clear field of view.

### 2. Insufficient power supply

**Problem:**  
During early testing, a 3xAA battery holder (4.5V total) was used. This voltage was too low for the system. The L298N motor driver introduces an internal voltage drop of about 2V, leaving too little voltage for the motors. At the same time, the ESP8266 regulator could not operate reliably.

**Solution:**  
The power source was replaced with batteries capable of delivering a higher voltage and current, above 7V in practice, ensuring stable regulator operation and sufficient motor power.

### 3. Servo instability and ESP8266 resets

**Problem:**  
Even after improving the power supply, the robot still restarted whenever the servo motor moved suddenly.

**Diagnosis:**  
This was identified as a **brownout** issue. The servo drew a high inrush current during startup, causing a temporary voltage drop below the ESP8266 operating threshold.

**Solution:**  
A **100µF electrolytic capacitor** was added to the 5V power line. It acts as an energy buffer, compensating for sudden voltage drops and stabilizing the microcontroller.

## Final Result and Conclusions

This project represented my first serious step into robotics. It was both a technical challenge and an excellent learning opportunity. I had to solve practical problems related to power delivery, wiring layout, movement control, turret control, and embedded programming.

Through patience and persistence, I managed to build a robot that can:

- move on command,
- communicate through a browser-based interface,
- detect nearby obstacles,
- enforce safety conditions before activating the laser.

More importantly, the project showed me how rewarding it is to see an idea become a working system. It also opened the door to future improvements such as:

- adding a camera for live video feedback,
- using more advanced sensors,
- implementing autonomous navigation,
- following predefined paths.

Overall, this project marks the beginning of my journey into robotics and embedded systems.

## Repository Structure

```text
.
├── README.md
└── robot_tank_targeting_system/
    └── robot_tank_targeting_system.ino
```
![WhatsApp Image 2026-03-31 at 18 14 35](https://github.com/user-attachments/assets/782d4ad5-113b-4b47-8a14-1e59bf6beb91)
![WhatsApp Image 2026-03-31 at 18 14 34 (1)](https://github.com/user-attachments/assets/43b2d4b6-e867-4fe5-b2f6-3816c0e8ffbe)

## Bibliography

- Espressif Systems – ESP8266 documentation
- STMicroelectronics – L298N datasheet
- SparkFun – HC-SR04 ultrasonic sensor datasheet
- Tower Pro – SG90 servo motor documentation
- Arduino Servo library reference
- ESP8266 Arduino documentation
#
