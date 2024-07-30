#include <esp_now.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address 0x27, 16 columns and 2 rows

uint8_t broadcastAddress1[] = { 0xEC, 0x64, 0xC9, 0x98, 0x79, 0x5C };  // Replace with your actual receiver MAC Address

const int buttonPin = 2; // 26 for Esp, 2 for Mini
bool buttonPressed = false;
unsigned long lastSendTime = 0;
const unsigned long sendTimeout = 15000;  // 15 seconds timeout
const int toAccept = 21; // 25 for Esp, 21 for Mini (red LED)
const int accepted = 20; // 33 for Esp, 20 for Mini (green LED)
const int sdaPin = 6; // 23 for Esp, 6 for Mini
const int sclPin = 7; // 19 for Esp, 7 for Mini

typedef struct struct_message {
  char message[32];
} struct_message;
xx
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

  digitalWrite(accepted, HIGH);
  digitalWrite(toAccept, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Acknowledged!");
}

void IRAM_ATTR handleButtonPress() {
  buttonPressed = true;
}

void setup() {
  // Initialize light pin
  pinMode(toAccept, OUTPUT);
  pinMode(accepted, OUTPUT);

  // initialize LCD
  Wire.begin(sdaPin, sclPin);  // SDA on GPIO 23, SCL on GPIO 19
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("This took TOO");
  lcd.setCursor(0, 1);
  lcd.print("long to make");

  // Init Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register for Send CB to get the status of transmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register for recv CB to get recv packet info
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer 1");
    return;
  }

  // Initialize button pin as input
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

      // Set message to send
      strcpy(myData.message, "LIGHT ON");

      // Send message via ESP-NOW to both peers
      esp_err_t result1 = esp_now_send(broadcastAddress1, (uint8_t *)&myData, sizeof(myData));

      if (result1 == ESP_OK) {
        Serial.println("Sent with success to peers");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Calling!");
        lcd.setCursor(0, 1);
        lcd.print("Wait a bit...");
      } else {
        Serial.println("Error sending the data");
      }
    } else {
      Serial.println("Timeout in effect. Please wait before sending again.");
    }
  }
}
