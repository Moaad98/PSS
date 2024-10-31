
# Personal Home Security System

This project is a simple home security system using ESP32 and ESP32-CAM modules. The ESP32 detects door status and triggers an alarm and a smart light bulb (Magic Home), while the ESP32-CAM captures images upon detecting unauthorized entry and uploads them to Firebase for remote access.

## Features

- **Door Monitoring**: Detects door status using a sensor connected to the ESP32.
- **Alert Mechanisms**: Activates a buzzer and smart light bulb when the door is opened.
- **Image Capture**: The ESP32-CAM captures images upon door opening and uploads them to Firebase for remote viewing.
- **WiFi Connectivity**: Connects to a WiFi network to control the light bulb and communicate with the ESP32-CAM.

## Hardware Requirements

1. ESP32
2. ESP32-CAM
3. Door Sensor
4. Buzzer
5. Magic Home-compatible smart light bulb

## Software Requirements

- Arduino IDE
- Firebase Account
- Firebase project setup with database and storage permissions
- Magic Home app for bulb setup (for testing)

## Code Overview

### ESP32 Code (Door and Bulb Controller)

- **WiFi Setup**: Connects to a specified WiFi network.
- **Smart Bulb Control**: Uses the `WifiLedBulb` class to control the Magic Home smart bulb via network commands.
- **Door Monitoring**: Reads input from the door sensor pin and triggers the buzzer and light.
- **ESP32-CAM Communication**: Sends a request to the ESP32-CAM when the door is opened to trigger image capture.

### ESP32-CAM Code (Image Capture and Upload)

- **Camera Setup**: Initializes the ESP32-CAM with the required pins and configurations.
- **Firebase Image Upload**: Uploads images to Firebase Storage.
- **Door Event Listener**: Listens for door open requests from ESP32 to capture and upload images.

## Setup Instructions

1. **WiFi Configuration**: Update `ssid` and `password` in both codes with your WiFi network credentials.
2. **Smart Bulb IP**: Replace `"Your Bulb IP"` with the IP address of your Magic Home bulb.
3. **Firebase Configuration**: Update the Firebase credentials (host, auth, project ID, and storage bucket) in the ESP32-CAM code.
4. **Camera Pin Configuration**: Adjust the camera pins in the ESP32-CAM code based on your ESP32-CAM model.

## Usage

1. Deploy the ESP32 and ESP32-CAM codes onto their respective devices.
2. Power on the ESP32 and ESP32-CAM, ensuring both are connected to the same WiFi network.
3. When the door opens, the ESP32 will:
   - Activate the buzzer and smart bulb.
   - Notify the ESP32-CAM to capture an image.
4. The ESP32-CAM will capture an image and upload it to Firebase for viewing.
