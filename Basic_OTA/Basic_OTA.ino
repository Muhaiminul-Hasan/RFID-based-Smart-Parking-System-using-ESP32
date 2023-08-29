/*
  Rui Santos
  Complete project details
   - Arduino IDE: https://RandomNerdTutorials.com/esp32-ota-over-the-air-arduino/
   - VS Code: https://RandomNerdTutorials.com/esp32-ota-over-the-air-vs-code/
  
  This sketch shows a Basic example from the AsyncElegantOTA library: ESP32_Async_Demo
  https://github.com/ayushsharma82/AsyncElegantOTA
*/

#include <Arduino.h>
#include <Wire.h>                    // I2C communication library
#include <LiquidCrystal_I2C.h>       // I2C LCD library
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <AsyncElegantOTA.h>

const char* ssid = "TP-Link_8320";      // your wifi hotspot name
const char* password = "Garighora";     // hotspot password

AsyncWebServer server(80);

// Address of I2C display
#define I2C_ADDR 0x27

// Initialize the display
LiquidCrystal_I2C lcd(I2C_ADDR, 16, 2);

void setup(void) {

  // Initialize the lcd 
  lcd.init();                      
  lcd.backlight();                            // Turn on the backlight
  lcd.print(F(" Smart  Parking"));            // Display initial message
  lcd.setCursor(0, 1);
  lcd.print(F("     System"));
  delay(2000);
  lcd.clear();

  // Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  // Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    // Serial.print(".");
    lcd.print(".");
  }
  // Serial.println("");
  // Serial.print("Connected to ");
  // Serial.println(ssid);
  // Serial.print("IP address: ");
  // Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.print(F("Connected to IP:"));
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP().toString());
  delay(1500);
  lcd.clear();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
  });

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  // Serial.println("HTTP server started");
}

void loop(void) {

}
