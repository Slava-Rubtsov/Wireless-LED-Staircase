# Wireless LED Staircase Controller

This project controls a **staircase LED lighting system** based on **motion detection** using **two ESP32 boards**.  
One ESP32 acts as the **LED controller and Bluetooth server**, and the second ESP32 acts as a **wireless PIR motion transmitter**.

Smooth animations light up the staircase when a person moves **up** or **down** the stairs, with automatic timeouts and graceful fades.

---

## 📋 Features

- **Wireless PIR sensors** via Bluetooth Serial
- **FastLED animations** for staircase lighting
- **Motion direction detection** (Up or Down)
- **Timeout with fading effect** if no second trigger detected
- **Automatic Bluetooth reconnection** if the wireless transmitter disconnects
- **Optimized multitasking** using FreeRTOS tasks (`xTaskCreatePinnedToCore`)
- **Failsafe behavior** in case of signal loss

---

## 🔌 Hardware Requirements

- 2 × ESP32 Development Boards
- 2 × PIR Motion Sensors (HC-SR501 or similar)
- 1 × WS2811 / WS2812B Addressable LED Strip
- Power supply for LEDs (12V with sufficient current)
- 2 × DC12V-DC%V converter

---

## ⚡ Wiring Overview

**LED Controller ESP32**:
- `GPIO 4` → Data input of LED strip
- `GPIO 25`  → PIR Sensor A

**Wireless PIR Transmitter ESP32**:
- `GPIO 25` → PIR Sensor B

---

## 🛠️ Software Requirements

- Arduino IDE with ESP32 support
- Libraries:
  - [`FastLED`](https://github.com/FastLED/FastLED)


