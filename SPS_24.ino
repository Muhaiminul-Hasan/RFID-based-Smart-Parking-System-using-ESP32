// Include necessary Libraries
  #include <SPI.h>                     // SPI communication library
  #include <MFRC522.h>                 // RFID/NFC reader library
  #include <Wire.h>                    // I2C communication library
  #include <LiquidCrystal_I2C.h>       // I2C LCD library
  #include <ServoESP32.h>              // Servo motor control library

  #include <Arduino.h>                 // Core Arduino functions
  #include <WiFi.h>                    // WiFi communication library
  #include <AsyncTCP.h>                // Asynchronous TCP communication library
  #include <ESPAsyncWebSrv.h>          // Asynchronous web server library
  #include <AsyncElegantOTA.h>         // Asynchronous over-the-air (OTA) update library
 
// Initialize WiFi and web server
  const char* ssid = "TP-Link_8320";   // your wifi hotspot name
  const char* password = "Garighora";  // hotspot password

  AsyncWebServer server(80);           // Create an asynchronous web server instance on port 80
 
// Pin Definitions
  
  // RFID SPI pins
    // MOSI   23
    // MISO   19
    // SCK    18

  // LCD I2C pins
    // SCL    22
    // SDA    21
  
  // Entry gate
  #define IR_SENSOR_ENTRY_VEHICLE_DETECT	  35

  #define LED_RED_1	     13 // for showing access denied
  #define LED_GREEN_1	   12 // for showing access granted
  #define BUZZER_1       14 // for access granted or deny sound

  #define SERVO_1	       32 // for opening and closing entry gate

  // RFID for Entry gate
  #define RFID_1_PIN_RST    33
  #define RFID_1_PIN_SS	    17

  // Exit gate
  #define IR_SENSOR_EXIT_VEHICLE_DETECT	    34

  #define LED_RED_2	     0  // for showing access denied
  #define LED_GREEN_2	   2  // for showing access granted
  #define BUZZER_2       15 // for access granted or deny sound

  #define SERVO_2	       4  // for opening and closing exit gate

  // RFID for Exit gate
  #define RFID_2_PIN_RST	  16
  #define RFID_2_PIN_SS	    5

  // Parking slots
  #define SONAR_SENSOR_SLOT_1   27  // parking slot 1
  #define SONAR_SENSOR_SLOT_2	  26  // parking slot 2
  #define SONAR_SENSOR_SLOT_3	  25  // parking slot 3
 
  // Initialize components
  // Address of I2C display
  #define I2C_ADDR 0x27

  // Initialize the display
  LiquidCrystal_I2C lcd(I2C_ADDR, 16, 2);

  // Initialize RFID modules
  MFRC522 rfid1(RFID_1_PIN_SS, RFID_1_PIN_RST);
  MFRC522 rfid2(RFID_2_PIN_SS, RFID_2_PIN_RST);

  // Initialize Servos
  Servo servo1;
  Servo servo2;
 
// Global variables
  // Parking slot
  volatile bool slot_1;
  volatile bool slot_2;
  volatile bool slot_3;

  // Define flags to indicate gate actions
  volatile bool openEntryGate = false;
  volatile bool openExitGate = false;

  // Store the card holder's name for display
  const char* cardHolderName;

  char entryOk [] = "Access Granted";
  char entryNotOk [] = "Access Denied";

// Index HTML
  const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
  <head>
    <title>Smart Parking System</title>
    <style>
      body {
        font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        text-align: center;
        background-color: rgb(70, 120, 200);
        margin: 0;
        padding: 0;
      }
      h1 {
        color: rgb(30, 30, 30);
        font-size: 4vw; /* Responsive font size based on viewport width */
        margin-bottom: 4vh; /* Responsive margin based on viewport height */
      }
      h2 {
        color: rgb(30, 30, 30);
        font-size: 3vw; /* Responsive font size based on viewport width */
        margin-bottom: 3vh; /* Responsive margin based on viewport height */
      }
      .slot {
        display: inline-block;
        margin: 2vh; /* Responsive margin based on viewport height */
        padding: 3vw; /* Responsive padding based on viewport width */
        background-color: rgb(192, 216, 251);
        border: 1px solid rgb(92, 184, 92);
        border-radius: 10px;
        box-shadow: 0 2px 5px rgba(0, 242, 255, 0.15);
        font-size: 2vw; /* Responsive font size based on viewport width */
      }
      .red {
        color: rgb(180, 13, 27);
      }
      .green {
        color: rgb(92, 184, 92);
      }
      @media (max-width: 768px) {
        .slot {
          margin: 2vh; /* Responsive margin based on viewport height */
          padding: 3vw; /* Responsive padding based on viewport width */
        }
        h1 {
          font-size: 3vw; /* Responsive font size based on viewport width */
          margin-bottom: 3vh; /* Responsive margin based on viewport height */
        }
      }
    </style>
  </head>
  <body>
    <h1>Smart Parking System</h1>
    <h2>Parking Slot Occupancy</h2>
    <div class="slot">
      <h2>Slot 1</h2>
      <p>Near Entrance</p>
      <p>Status: <span id="slot1" class="status">Unknown</span></p>
      <p>Occupant: <span id="Occupant1" class="Occupant">Unknown</span></p>
    </div>
    <div class="slot">
      <h2>Slot 2</h2>
      <p>Central Area</p>
      <p>Status: <span id="slot2" class="status">Unknown</span></p>
      <p>Occupant: <span id="Occupant2" class="Occupant">Unknown</span></p>
    </div>
    <div class="slot">
      <h2>Slot 3</h2>
      <p>Near Exit</p>
      <p>Status: <span id="slot3" class="status">Unknown</span></p>
      <p>Occupant: <span id="Occupant3" class="Occupant">Unknown</span></p>
    </div>
    <p>Last Update: <span id="timestamp">Unknown</span></p>
    <script>
      async function updateOccupancy() {
        const response = await fetch('/occupancy');
        const data = await response.json();
        document.getElementById('slot1').innerText = data.slot1 ? 'Occupied' : 'Vacant';
        document.getElementById('slot1').className = data.slot1 ? 'red' : 'green';

        document.getElementById('Occupant1').innerText = data.slot1 ? 'Nowrin' : 'not present';

        document.getElementById('slot2').innerText = data.slot2 ? 'Occupied' : 'Vacant';
        document.getElementById('slot2').className = data.slot2 ? 'red' : 'green';

        document.getElementById('Occupant2').innerText = data.slot2 ? 'Aliya' : 'not present';

        document.getElementById('slot3').innerText = data.slot3 ? 'Occupied' : 'Vacant';
        document.getElementById('slot3').className = data.slot3 ? 'red' : 'green';

        document.getElementById('Occupant3').innerText = data.slot3 ? 'Sanam' : 'not present';
        
        const now = new Date();
        const formattedTime = now.toLocaleTimeString();
        document.getElementById('timestamp').innerText = formattedTime;
      }
      setInterval(updateOccupancy, 1000); // Update occupancy every 1 second
    </script>
  </body>
  </html>
  )rawliteral";


// main setup function(mendatory for any arduino code)
void setup() {
  // Initialize I2C proltocol bus for LCD module
  Wire.begin();
  delay(20);

  // Initialize the lcd 
  lcd.init();                      
  lcd.backlight();                      // Turn on the backlight
  lcd.print(F(" Smart  Parking"));      // Display initial message
  lcd.setCursor(0, 1);
  lcd.print(F("     System"));
  delay(2000);
  lcd.clear();

  // Initialize WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  delay(500);

  // Attempt to connect to WiFi
  int attemptLeft = 5;
  bool wifiConnected = false;
  
  
  while (attemptLeft > 0) {               // Wait for connection
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;               // Update wifiConnected when connection is successful
      break;
    }
    delay(500);
    lcd.print(".");
    attemptLeft--;
  }
  lcd.clear();
  // Start server if WiFi Connection successful
  if (wifiConnected) {
    lcd.print(F("Connected to IP:"));
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP().toString());
    delay(2000);
    lcd.clear();
    
    // Handle root URL and send a simple response
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      // request->send(200, "text/plain", "Hi! I am ESP32.");
      request->send(200, "text/html", index_html);
    });
    
    server.on("/occupancy", HTTP_GET, [](AsyncWebServerRequest *request) {
      // Create a JSON response object with slot occupancy data
      const String response = String("{\"slot1\":") + (slot_1 ? "true" : "false") +
                              String(",\"slot2\":") + (slot_2 ? "true" : "false") +
                              String(",\"slot3\":") + (slot_3 ? "true" : "false") +
                              String("}");
      request->send(200, "application/json", response);
    });

    // Start ElegantOTA
    AsyncElegantOTA.begin(&server);    // Start ElegantOTA for Over-The-Air updates
    delay(20);
    server.begin();
    delay(20);

    delay(200);
    lcd.print("Server started");
    delay(1000);
    lcd.clear();
  } else {
  WiFi.disconnect();                   // Disconnect WiFi if connection attempt fails
    WiFi.mode(WIFI_OFF);               // Turn off WiFi
  }

  // Initialize SPI protocol bus for RFID modules
  SPI.begin();
  delay(20);

  delay(200);
  lcd.print("SPI begun");
  delay(1000);
  lcd.clear();

  // Initialize RFID Modules
  rfid1.PCD_Init(); // Initialize RFID Module 1
  rfid2.PCD_Init(); // Initialize RFID Module 2

  delay(200);
  lcd.print("RFID OK");
  delay(1000);
  lcd.clear();

  // Set thr IR sensor pins as input with pullup(because the IR sensor tells the program that a car is detected by pulling the 'out' pin LOW/ 0)
  pinMode(IR_SENSOR_ENTRY_VEHICLE_DETECT, INPUT_PULLUP);
  pinMode(IR_SENSOR_EXIT_VEHICLE_DETECT, INPUT_PULLUP);

  // Set the LEDs and buzzer pins as output
  pinMode(LED_RED_1, OUTPUT);
  pinMode(LED_GREEN_1, OUTPUT);
  pinMode(BUZZER_1, OUTPUT);

  pinMode(LED_RED_2, OUTPUT);
  pinMode(LED_GREEN_2, OUTPUT);
  pinMode(BUZZER_2, OUTPUT);

  // Initialize Servos
  servo1.attach(SERVO_1);   // Attach servo to pin
  servo2.attach(SERVO_2);   // Attach servo to pin

  // Close the gates when program first starts
  servo1.write(90);         // close Entry gate
  delay(250);
  servo2.write(90);         // close Exit gate
  delay(250);
  
  // EntryGateTask to run on core 0
  xTaskCreatePinnedToCore(
    EntryGateTask,             // Task function.
    "EntryGateTask",           // name of task.
    10000,                     // Stack size of task
    NULL,                      // parameter of the task
    3,                         // priority of the task
    NULL,                      // Task handle to keep track of created task
    !ARDUINO_RUNNING_CORE);    // pin task to core

  // ExitGateTask to run on core 0
  xTaskCreatePinnedToCore(
    ExitGateTask,              // Task function.
    "ExitGateTask",            // name of task.
    10000,                     // Stack size of task
    NULL,                      // parameter of the task
    2,                         // priority of the task
    NULL,                      // Task handle to keep track of created task
    !ARDUINO_RUNNING_CORE);    // pin task to core

  // UpdateDisplayTask to run on core 1
  xTaskCreatePinnedToCore(
    UpdateDisplayTask,         // Task function.
    "UpdateDisplayTask",       // name of task.
    10000,                     // Stack size of task
    NULL,                      // parameter of the task
    1,                         // priority of the task
    NULL,                      // Task handle to keep track of created task
    ARDUINO_RUNNING_CORE);     // pin task to core
}


// main loop function(mendatory for any arduino code)
void loop() {
  // Handle vehicle on entry gate
  if (digitalRead(IR_SENSOR_ENTRY_VEHICLE_DETECT) == LOW) {    // check if Car is present on Entry gate
    handleVehiclePresentEntry();                               // call this function to handle Entry event
  }
  // Handle vehicle on exit gate
  if (digitalRead(IR_SENSOR_EXIT_VEHICLE_DETECT) == LOW) {     // check if Car is present on Exit gate
    handleVehiclePresentExit();                                // call this function to handle Exit event
  }
}


// Helper functions used are defined down below


// Function to check if parking slot is occupied or not
bool isSlotOccupied(int trigPin) {
  float distance;
  
  pinMode(trigPin, OUTPUT);                  // set the pin as output to trigger pulse
  
  digitalWrite(trigPin, HIGH);               // trigger a pulse to the ultrasonic sensor
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  pinMode(trigPin, INPUT);                   // set the pin as input to receive the echo
  
  distance = pulseIn(trigPin, HIGH);         // measure the time it takes for the sound to reflect back
  distance = distance * 340 / 2 / 10000;     // multiply the time by speed of sound (340 km/h)
                                             // divide by two because of two way round trip
                                             // divide by 10000 to convert kilometer to centemeter
  
  pinMode(trigPin, OUTPUT);
  
  delay(20);
  
  return (distance < 4) ? true : false;      // return true if slot is occupied and false if vacant
}

// Function to handle Entry event
void handleVehiclePresentEntry() {
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid1.PICC_IsNewCardPresent())                       // check if card is placed over the RFID scanner 
    return;

  // Verify if the NUID has been readed
  if ( ! rfid1.PICC_ReadCardSerial())                         // read the card/ tag number
    return;
  
  if (isAllowedCard(rfid1.uid.uidByte, rfid1.uid.size)) {     // check if card is allowed or not
    // case 1: Access granted
    rfid1.PICC_HaltA();
    rfid1.PCD_StopCrypto1();
    
    // Open Entry gate
    openEntryGate = true;

  } else {
    // case 2: Access denied
    // Trigger LED
    digitalWrite(LED_RED_1, HIGH);    // Turn RED LED ON

    lcd.clear();                      // clear LCD screen before displaying anything
    lcd.noBlink();
    lcd.print(F(entryNotOk));         // Print Access Denied on LCD
    
    // Sound Buzzer
    accessDeniedSound(BUZZER_1);
    
    delay(500);
    lcd.clear();                      // clear LCD screen
    digitalWrite(LED_RED_1, LOW);     // Turn RED LED OFF
    slot_1 = !slot_1;                 // Update LCD
  }
}


// Function to handle Exit event
void handleVehiclePresentExit() {
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid2.PICC_IsNewCardPresent())                       // check if card is placed over the RFID scanner 
    return;

  // Verify if the NUID has been readed
  if ( ! rfid2.PICC_ReadCardSerial())                         // read the card/ tag number
    return;
  
  if (isAllowedCard(rfid2.uid.uidByte, rfid2.uid.size)) {     // check if card is allowed or not
    // Access granted

    // Deactivate read tag to prevent reading it again
    rfid2.PICC_HaltA();                 // Deactivate the current tag's communication
    rfid2.PCD_StopCrypto1();            // Stop cryptographic communication with the tag
    
    // Open Exit gate
    openExitGate = true;

  } else {
    // case 2: Access denied
    // Trigger LED
    digitalWrite(LED_RED_2, HIGH);      // Turn RED LED ON
    
    // Sound Buzzer
    accessDeniedSound(BUZZER_2);

    delay(500);
    digitalWrite(LED_RED_2, LOW);       // Turn RED LED OFF
  }
}

// Card Info Structure
struct CardInfo {
  byte uid[4];
  const char* name;
};

// Card Database
CardInfo allowedCards[] = {
  {{0xD3, 0x2F, 0x75, 0x1E}, "Nowrin"},
  {{0xE3, 0x85, 0x7C, 0x1E}, "Aliya"},
  {{0x43, 0x17, 0x9C, 0xAA}, "Sanam"},
  // {{0x53, 0x57, 0xD4, 0xAA}, "D"},
  {{0xF3, 0x09, 0xA4, 0x9A}, "Nowrin"},
  {{0x93, 0x66, 0x5F, 0xA6}, "Aliya"},
};

// function to check if RFID tag is on the allowed list
bool isAllowedCard(byte *uid, byte uidSize) {
  for (const auto& card : allowedCards) {
    if (memcmp(uid, card.uid, uidSize) == 0) {
      cardHolderName = card.name;
      return true; // Access allowed
    }
  }
  return false; // Access denied
}

// Entry gate open/close Task
void EntryGateTask(void * parameters) {
  for(;;) {
    if (openEntryGate) {
    
      digitalWrite(LED_GREEN_1, HIGH);  // Turn GREEN LED ON
      lcd.clear();
      lcd.noBlink();
      lcd.print(F(entryOk));            // Print Access Granted on LCD
      
      delay(500);
      lcd.clear();
      delay(200);
      lcd.print(F("Welcome, "));
      lcd.print(cardHolderName);
      lcd.print(F("."));

      // Sound Buzzer
      accessGrantedSound(BUZZER_1);

      // Open Entry Gate
      controlGate(servo1, 1);           // 1 = open, 0 = close

      digitalWrite(LED_GREEN_1, LOW);   // Turn GREEN LED OFF
      delay(2500);
      
      // Close Entry Gate
      controlGate(servo1, 0);           // 1 = open, 0 = close
      lcd.clear();
      slot_1 = !slot_1;                 // Update LCD
      openEntryGate = false;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// Exit gate open/close Task
void ExitGateTask(void * parameters) {
  for(;;) {
    if (openExitGate) {
    
      digitalWrite(LED_GREEN_2, HIGH);  // Turn GREEN LED ON

      // Sound Buzzer
      accessGrantedSound(BUZZER_2);
      // Open Exit Gate
      controlGate(servo2, 1);           // 1 = open, 0 = close

      digitalWrite(LED_GREEN_2, LOW);   // Turn GREEN LED OFF
      delay(2500);

      // Close Exit Gate
      controlGate(servo2, 0);           // 1 = open, 0 = close
      openExitGate = false;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// Update Display Task
void UpdateDisplayTask(void * parameters) {
  for(;;) {
    slot_1 = isSlotOccupied(SONAR_SENSOR_SLOT_1);                // check if slot 1 is occupied
    delay(10);
    slot_2 = isSlotOccupied(SONAR_SENSOR_SLOT_2);                // check if slot 2 is occupied
    delay(10);
    slot_3 = isSlotOccupied(SONAR_SENSOR_SLOT_3);                // check if slot 3 is occupied
    delay(10);

    updateDisplay(slot_1, slot_2, slot_3);                       // update parking slot occupancy on LCD]
  }
  vTaskDelay(500 / portTICK_PERIOD_MS);
}

// update display for parking slot occupancy
void updateDisplay(bool slot1, bool slot2, bool slot3) {
  static bool prevSlot1 = true;
  static bool prevSlot2 = true;
  static bool prevSlot3 = true;

  if (slot1 != prevSlot1 || slot2 != prevSlot2 || slot3 != prevSlot3) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  S1   S2   S3  ");            // Slot 1, 2 and 3
    
    lcd.setCursor(0, 1);
    lcd.print(slot1 ? " used" : " free");     // print 'used' if slot occupied, print 'free' if slot is vacant/empty
    lcd.print(slot2 ? " used" : " free");
    lcd.print(slot3 ? " used" : " free");
    
    prevSlot1 = slot1;
    prevSlot2 = slot2;
    prevSlot3 = slot3;
  }
}

// Function to Open and Close gates
void controlGate(Servo &servo, int direction) {
  int pos = (direction == 1) ? 90 : 0;        // Set position based on direction
  int targetPos = (direction == 1) ? 0 : 90;  // Set target position based on direction
  
  int step = (targetPos > pos) ? 1 : -1;      // Determine the step direction
  
  for (; pos != targetPos; pos += step) {
    servo.write(pos);                         // Set the servo to the new position
    delay(15);                                // Small delay to allow the servo to move
  }
}

// Function to generate buzzer sound
void generateTone(int pin, unsigned int frequency, unsigned long duration) {
  unsigned long period = 1000000 / frequency;       // Calculate the period of one cycle in microseconds
  unsigned long startTime = micros();               // Get the current time

  while (micros() - startTime < duration * 1000) {  // Convert duration to microseconds
    digitalWrite(pin, HIGH);                        // Turn on the buzzer
    delayMicroseconds(period / 2);                  // Wait for half of the period
    digitalWrite(pin, LOW);                         // Turn off the buzzer
    delayMicroseconds(period / 2);                  // Wait for the other half of the period
  }
}

// Function to generate access granted buzzer sound
void accessGrantedSound(int pin) {
  int frequency = 1400;
  int duration = 100;
  generateTone(pin, frequency, duration);  // Frequency, duration
  delay(50);  // Pause between tones
  generateTone(pin, frequency, duration);
  delay(50);
}

// Function to generate access denied buzzer sound
void accessDeniedSound(int pin) {
  int frequency = 400;
  int duration = 100;
  generateTone(pin, frequency, duration);  // Frequency, duration
  delay(50);  // Pause between tones
  generateTone(pin, frequency, duration);
  delay(50);
}