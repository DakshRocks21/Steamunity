#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize the I2C LCD (address 0x27, 16 columns, 2 rows)
LiquidCrystal_I2C lcd(0x27, 16, 2);

uint8_t rangeExtenderAddress[] = {0xEC,0x64,0xc9,0x98,0x7e,0x10};  /* Range extender MAC address */ 

const int buttonPin = 6; // 26 for Esp, 6 for Mini
bool buttonPressed = false;
unsigned long lastSendTime = 0;

const unsigned long sendTimeout = 2000;  // 15 seconds timeout
const int toAccept = 10; // 25 for Esp, 10 for Mini (red LED)
const int accepted = 7; // 33 for Esp, 7 for Mini (green LED)
const int sdaPin = 4;
const int sclPin = 5;

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
    lcd.setCursor(0, 0);
    lcd.print("Acknowledged!");
  } else if (strcmp(receivedData.message, "FAILED") == 0) {
    digitalWrite(toAccept, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Send Failed!");
  }
}

void IRAM_ATTR handleButtonPress() {
  buttonPressed = true;
}

void setup() {
  pinMode(toAccept, OUTPUT);
  pinMode(accepted, OUTPUT);

  Wire.begin(sdaPin, sclPin);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("This took TOO");
  lcd.setCursor(0, 1);
  lcd.print("long to make");

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
    digitalWrite(toAccept, HIGH);

    unsigned long currentTime = millis();
    if (currentTime - lastSendTime >= sendTimeout) {
      lastSendTime = currentTime;

      strcpy(myData.message, "LIGHT ON");

      esp_err_t result = esp_now_send(rangeExtenderAddress, (uint8_t *)&myData, sizeof(myData));

      if (result == ESP_OK) {
        Serial.println("Sent with success to range extender");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Calling!");
        lcd.setCursor(0, 1);
        lcd.print("Wait a bit...");
      } else {
        Serial.println("Error sending the data");
        digitalWrite(toAccept, LOW);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Send Failed!");
      }
    } else {
      Serial.println("Timeout in effect. Please wait before sending again.");
    }
  }
}
