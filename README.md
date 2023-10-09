# RFID-based-Smart-Parking-System-using-ESP32
This project is an RFID-based Smart Parking System implemented using an ESP32 microcontroller.
![IMG_20230826_151135_525](https://github.com/Muhaiminul-Hasan/RFID-based-Smart-Parking-System-using-ESP32/assets/50764001/16582899-4241-4595-9542-c45b30e224dc)


![IMG_20230826_151035_058](https://github.com/Muhaiminul-Hasan/RFID-based-Smart-Parking-System-using-ESP32/assets/50764001/1d8434ea-38ed-46b1-afe7-a22fd31f1c89)

Features
RFID scanner on both the Entry gate and Exit gate
Utilizes the freeRTOS Real-Time Operating System
Local webserver with a webpage on WiFi LAN to show parking slot allocations
Auto refreshes the webpage
Supports firmware updates via OTA (Over-the-Air)
Getting Started
To get started with the RFID-based Smart Parking System, follow these steps:

Clone or download the project repository.
Set up the necessary hardware components, including the ESP32 microcontroller and RFID scanners.
Upload the firmware to the ESP32 using your preferred method (e.g., Arduino IDE, PlatformIO).
Connect the ESP32 to your local WiFi network.
Access the local webserver by entering the ESP32's IP address in a web browser.
The webpage will display the parking slot allocations in real-time.
Dependencies
This project relies on the following dependencies:

freeRTOS: Real-Time Operating System for ESP32
ESPAsyncWebServer: Asynchronous Web Server Library for ESP32
MFRC522: RFID Library for MFRC522 RFID scanner module
Usage
Once the RFID-based Smart Parking System is set up and running, it will automatically scan RFID tags at the Entry and Exit gates to manage parking slot allocations. The local webserver will display the current parking slot allocations on a webpage, which will be automatically refreshed.

Firmware Updates
The ESP32 microcontroller supports Over-the-Air (OTA) firmware updates. To update the firmware, follow these steps:

Ensure that the ESP32 is connected to the local WiFi network.
Access the ESP32's IP address in a web browser.
Navigate to the firmware update section on the webpage.
Choose the firmware file to upload and initiate the update process.
The ESP32 will download and install the new firmware over the air.
License
This project is licensed under the MIT License.

Acknowledgments
Special thanks to the contributors and developers of the freeRTOS, ESPAsyncWebServer, and MFRC522 libraries.

For more details and information, please refer to the project rubric.

Feel free to reach out if you have any questions or need further assistance!

Happy coding!
