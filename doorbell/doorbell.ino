#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize the I2C LCD (address 0x27, 16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

uint8_t rangeExtenderAddress[] = { 0x08, 0xf9, 0xe0, 0xf6, 0xe9, 0xe4 }; // Range extender MAC address

bool buttonPressed = false;
bool displayAcknowledged = false;
unsigned long lastSendTime = 0;
unsigned long ackDisplayStartTime = 0;

const unsigned long sendTimeout = 2000;  // 2 seconds timeout
const unsigned long ackDisplayDuration = 15000; // 15 seconds timeout for "Acknowledged" display
const int buttonPin = D6; // 26 for ESP, D6 for Mini
const int toAccept = D10; // 25 for ESP, D10 for Mini (red LED)
const int accepted = D7; // 33 for ESP, D7 for Mini (green LED)
const int sdaPin = D4; // 23 for ESP, D4 for Mini
const int sclPin = D5; // 19 for ESP, D5 for Mini

typedef struct struct_message {
  char message[32];
} struct_message;

struct_message myData;
struct_message receivedData;

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Message: ");
  Serial.println(receivedData.message);
  Serial.println();

  if (strcmp(receivedData.message, "ACK_1") == 0) {
    digitalWrite(accepted, HIGH);
    digitalWrite(toAccept, LOW);
    lcd.clear();
    lcd.init();
    lcd.setCursor(0, 0);
    lcd.print("Acknowledged!");
    displayAcknowledged = true;
    ackDisplayStartTime = millis();  // Record the time when ACK is shown
  } else if (strcmp(receivedData.message, "FAILED") == 0) {
    digitalWrite(toAccept, LOW);
    lcd.clear();
    lcd.init();
    lcd.setCursor(0, 0);
    lcd.print("Send Failed!");
  }
}

volatile unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 500; // Debounce delay in milliseconds

void IRAM_ATTR handleButtonPress() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastDebounceTime > debounceDelay) {
    buttonPressed = true; 
    lastDebounceTime = currentTime; 
  }
}

void setup() {
  pinMode(toAccept, OUTPUT);
  pinMode(accepted, OUTPUT);

  Wire.begin(sdaPin, sclPin);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.init();
  lcd.setCursor(0, 0);
  lcd.print("Presenting DUO");
  lcd.setCursor(0, 1);
  lcd.print("Group 12A");

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  memcpy(peerInfo.peer_addr, rangeExtenderAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButtonPress, FALLING);
}

void loop() {
  if (buttonPressed) {
    buttonPressed = false;

    unsigned long currentTime = millis();
    if (currentTime - lastSendTime >= sendTimeout) {
      lastSendTime = currentTime;

      strcpy(myData.message, "LIGHT ON");

      esp_err_t result = esp_now_send(rangeExtenderAddress, (uint8_t *)&myData, sizeof(myData));

      if (result == ESP_OK) {
        Serial.println("Sent with success to range extender");
        lcd.clear();
        lcd.init();
        lcd.setCursor(0, 0);
        lcd.print("Calling!");
        lcd.setCursor(0, 1);
        lcd.print("Wait a bit...");
      } else {
        Serial.println("Error sending the data");
        digitalWrite(toAccept, LOW);
        lcd.clear();
        lcd.init();
        lcd.setCursor(0, 0);
        lcd.print("Send Failed!");
      }
    } else {
      Serial.println("Timeout in effect. Please wait before sending again.");
      lcd.clear();
      lcd.init();
      lcd.setCursor(0, 0);
      lcd.print("Timeout :(");
      lcd.setCursor(0, 1);
      lcd.print("Don't Spam!");
    }
  }

  // Check if the "Acknowledged" message should be reverted back to the default after 15 seconds
  if (displayAcknowledged && millis() - ackDisplayStartTime >= ackDisplayDuration) {
    lcd.clear();
    lcd.init();
    lcd.setCursor(0, 0);
    lcd.print("Presenting DUO");
    lcd.setCursor(0, 1);
    lcd.print("Group 12A");
    displayAcknowledged = false; // Reset the flag
    digitalWrite(accepted, LOW); // Turn off the accepted LED
  }
}
