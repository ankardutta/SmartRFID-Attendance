# Power-efficient time-scheduled scanning accessed SmartRFID-Attendance system

SmartRFID-Attendance is an IoT-based attendance management system that leverages RFID technology and NodeMCU (ESP8266) to automate and streamline the process of recording attendance. The system reads RFID cards, logs attendance data, and sends it securely to a Google Sheet for centralized record-keeping. This project is ideal for educational institutions, offices, and other organizations seeking a reliable and contactless attendance solution.

## Features

- **Contactless Attendance:** Uses RFID cards for quick and hygienic attendance marking.
- **Real-Time Data Logging:** Attendance data is sent instantly to a Google Sheet via HTTPS.
- **Time Slot Recognition:** Automatically identifies entry, lunch, and exit slots based on real-time clock (NTP).
- **LCD Display:** Provides user feedback and system status on a 16x2 I2C LCD.
- **WiFi Connectivity:** Connects to WiFi for cloud integration and time synchronization.
- **Buzzer Alerts:** Audible feedback for successful or denied scans.
- **Secure Communication:** Utilizes HTTPS for secure data transmission.
- **Frame Capture:** Capture the users frame while scanning the cards.
## System Overview

![Project Demonstration](https://github.com/user-attachments/assets/846083c7-74d6-45be-80fc-47cd09e6577a)

The above image demonstrates the working prototype, including the NodeMCU, RFID reader, LCD display, and buzzer.

## Block Diagram

![Block Diagram](https://github.com/user-attachments/assets/3e5c7690-e6ad-4d5f-a161-3eb47825ba2d)

This block diagram illustrates the system architecture, showing the connections between the NodeMCU, RFID module, LCD, buzzer, and cloud services.

## How It Works

1. **Initialization:** The system connects to WiFi and synchronizes time using NTP.
2. **RFID Scan:** When a user scans their RFID card, the UID is read.
3. **Time Slot Detection:** The system determines if the scan is for entry, lunch, or exit based on the current time.
4. **Data Logging:** The UID and time slot are sent to a Google Sheet via a secure web request.
5. **User Feedback:** The LCD displays the scan result, and the buzzer provides audible confirmation.

## Getting Started

### Hardware Requirements

- NodeMCU (ESP8266)
- MFRC522 RFID Reader
- RFID Cards/Tags
- 16x2 I2C LCD Display
- ESP32 CAM
- Buzzer
- Connecting Wires

### Software Requirements

- Arduino IDE
- Required Libraries:
  - `ESP8266WiFi`
  - `ESP8266HTTPClient`
  - `MFRC522`
  - `NTPClient`
  - `WiFiUdp`
  - `LiquidCrystal_I2C`
  - `EloquentEsp32cam`
  - `Wire`

### Setup Instructions

1. **Clone this repository.**
2. **Open `sketch_may29blol.ino` in Arduino IDE.**
3. **Install all required libraries via Library Manager.**
4. **Update WiFi credentials and Google Script URL in the code.**
5. **Upload the code to your NodeMCU**
6. **Connect the hardware as per the block diagram.**
7. **Scan RFID cards to record attendance.**
8. **Now open `write_rfid_nodemcu.ino` file to write information onto RFID Tags and cards.**
9. **Time scheduling scanning mechanism applied here**

## Author & Acknowledgements

This project was implemented and guided by **Subhanakr Dutta**.  
Special thanks to all contributors and open-source library authors whose work made this project possible.

---

For any queries or contributions, please open an issue or submit a pull request.
