# 🧊 Smart Refrigerator Tray (IoT Project)

## 📖 Project Overview
This project is an IoT-enabled smart refrigerator tray designed to autonomously track inventory. Utilizing weight-based sensing, the system calculates the exact count of a specific item resting on the tray. If the inventory drops below a predefined threshold, the system triggers a remote notification to alert the user.
This project was developed for the Advanced Networks course.

## 🔗 Live Demo
Web App: https://smart-fridge-tray-production.up.railway.app

## 🏗️ System Architecture
The system operates on a WAN scale, structured across three primary phases:

1. **Hardware / Edge Node (ESP32):** Reads raw analog data from the load cell, amplifies it via the HX711, calculates the item count, and connects to the local Wi-Fi.
2. **Communication Protocol (MQTT):** A lightweight publish-subscribe protocol chosen to efficiently transmit the calculated item count over the global internet with minimal bandwidth.
3. **Backend & Client (Node.js + Web App):** A custom-hosted backend server receives the MQTT payloads, evaluates stock thresholds, handles alerts, and serves a custom Web Dashboard GUI for the user to view live inventory.

## 📡 Communication Flow
ESP32 → MQTT → Node.js Server (Railway) → WebSocket → Web App

## ⚙️ Hardware Components
- Microcontroller: ESP32 Development Board (Built-in Wi-Fi)
- Sensor: 5kg Straight Bar Load Cell + HX711 24-bit ADC Amplifier
- Outputs: ESP32 Built-in LED (Network Status Indicator)
- Maquette: Custom-built suspended tray mechanism

## 💻 Software Stack
- Embedded: C/C++ (Arduino IDE)
- IoT Protocol: MQTT
- Backend: Node.js / Express
- Frontend GUI: HTML / CSS / JavaScript

## 📚 Libraries Used
- HX711 by Bogde (Arduino)
- PubSubClient by Nick O'Leary (Arduino)
- ArduinoJson by Benoit Blanchon (Arduino)
- Express (Node.js)
- Aedes MQTT Broker (Node.js)
- ws WebSocket (Node.js)

## 👥 Team Members
- Youssef Khaled
- Antwan Micheal
- Omar Deyaa
