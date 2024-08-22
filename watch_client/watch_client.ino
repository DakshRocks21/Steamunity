#include <esp_now.h>
#include <WiFi.h>
#include "pitches.h"

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = { 0xEC, 0x64, 0xC9, 0x98, 0x7E, 0x10 };  // Daksh' Board
// uint8_t broadcastAddress[] = {0x08, 0xF9, 0xE0, 0xF6, 0xE9, 0xE4}; // NOT Daksh' Board, WC Board
//uint8_t broadcastAddress[] = {0xEC, 0x64, 0xC9, 0x98, 0x79, 0x5C}; // NOT Daksh' Board, Richards Board

int melody[] = {
  // NOTE_C4, 0, NOTE_G4, 0, NOTE_AS4, NOTE_C5, NOTE_AS4, 0, NOTE_F4, NOTE_DS4, 0,
  // NOTE_C4, 0, NOTE_G4, 0, NOTE_AS4, NOTE_C5, NOTE_AS4, 0, NOTE_F4, NOTE_DS4, 0,
  // NOTE_C4, 0, NOTE_G4, 0, NOTE_AS4, NOTE_C5, NOTE_AS4, 0, NOTE_F4, NOTE_DS4, 0,

  NOTE_C4, 0, NOTE_E4, 0, NOTE_G4, NOTE_A4, NOTE_AS4,
  NOTE_C5, 0, NOTE_C5, 0, NOTE_AS4, 0, NOTE_A4, 0,
  NOTE_AS4, 0, NOTE_AS4, NOTE_C5, 0, NOTE_AS4, NOTE_A4, 0,
  0,
  NOTE_C5, 0, NOTE_AS4, 0, NOTE_A4, 0, NOTE_AS4, 0, NOTE_E5,
  0,

  NOTE_C5, 0, NOTE_C5, 0, NOTE_AS4, 0, NOTE_A4, 0,
  NOTE_AS4, 0, NOTE_AS4, NOTE_C5, 0, NOTE_AS4, NOTE_A4, 0,
  0,
  NOTE_C5, 0, NOTE_AS4, 0, NOTE_A4, 0, NOTE_AS4, 0, NOTE_E4, 0,
};

int noteDurations[] = {
  // 4, 8, 4, 8, 4, 8, 8, 16, 8, 8, 16,
  // 4, 8, 4, 8, 4, 8, 8, 16, 8, 8, 16,
  // 4, 8, 4, 8, 4, 8, 8, 16, 8, 8, 16,

  4, 8, 4, 8, 4, 4, 4,
  8, 16, 8, 16, 8, 16, 8, 16,
  8, 16, 8, 8, 16, 8, 8, 16,
  4,
  8, 16, 8, 16, 8, 16, 8, 4, 8,
  4,

  8, 16, 8, 16, 8, 16, 8, 16,
  8, 16, 8, 8, 16, 8, 8, 16,
  4,
  8, 16, 8, 16, 8, 16, 8, 4, 8, 2
};

// ESP_TINY
// Tested on a big board - to be modified
// Button - 12
// LED - 26
// Buzzer 25

// Defining Pins
const int buttonPin = 9; //12 for ESP, 9 for Mini
const int ledPin = 3; //26 for ESP, 3 for Mini
const int buzzerPin = 25; //25 for ESP, 10 for Mini
const int vibrationPin = 8 //New addition for Mini
bool buttonPressed = false;
bool lightOnReceived = false;

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char message[32];
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Create a struct_message to receive data
struct_message receivedData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// callback function that will be executed when data is received
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Message: ");
  Serial.println(receivedData.message);
  Serial.println();

  if (strcmp(receivedData.message, "LIGHT ON") == 0) {
    lightOnReceived = true;
    // digitalWrite(25, HIGH); // Turn on buzzer
    digitalWrite(ledPin, HIGH);  // Turn on light
  }
}

void IRAM_ATTR handleButtonPress() {
  buttonPressed = true;
}

void setup() {
  // lightPin and buzzerPin
  pinMode(buzzerPin, OUTPUT);  // Buzzer
  pinMode(ledPin, OUTPUT);     // Light

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
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  // Initialize button pin as input
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButtonPress, FALLING);
}

void loop() {
  if (buttonPressed && lightOnReceived) {
    buttonPressed = false;
    lightOnReceived = false;

    // Set response message
    strcpy(myData.message, "ACK");

    // Send response message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

    if (result == ESP_OK) {
      Serial.println("ACK sent with success");
    } else {
      Serial.println("Error sending the ACK");
    }
  }
  if (lightOnReceived) {
    for (int thisNote = 0; thisNote < sizeof(melody) / sizeof(melody[0]); thisNote++) {
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(buzzerPin, melody[thisNote], noteDuration);

      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(buzzerPin);
      if (buttonPressed) {
        digitalWrite(ledPin, LOW);  // Turn off light
        noTone(buzzerPin);          // Turn off buzzer
        break;
      }
    }
  }
  // delay(500);
}