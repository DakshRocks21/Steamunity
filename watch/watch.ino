#include <esp_now.h>
#include <WiFi.h>
#include "pitches.h"

uint8_t rangeExtenderAddress[] = { 0x08, 0xf9, 0xe0, 0xf6, 0xe9, 0xe4 }//mini{0xEC,0x64,0xc9,0x98,0x7e,0x10}; /* Range extender MAC address */ 

int melody[] = {
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

// Defining Pins - for the Mini, 2, 8 and 9 are reserved for boot modes -> avoid using
const int buttonPin = 12; //12 for ESP, 0 for Mini
const int ledPin = 26; //26 for ESP, 10 for Mini
const int buzzerPin = 25; //25 for ESP, 3 for Mini
//const int motorPin = 1; //New addition for Mini
bool buttonPressed = false;
bool lightOnReceived = false;

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

  if (strcmp(receivedData.message, "LIGHT ON") == 0) {
    lightOnReceived = true;
    digitalWrite(ledPin, HIGH);
    digitalWrite(motorPin, HIGH);  // Turn on the vibration motor
  }
}

void IRAM_ATTR handleButtonPress() {
  buttonPressed = true;
}

void setup() {
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(motorPin, OUTPUT); // Set vibration motor pin as output

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
  if (buttonPressed && lightOnReceived) {
    buttonPressed = false;
    lightOnReceived = false;

    strcpy(myData.message, "ACK");

    esp_err_t result = esp_now_send(rangeExtenderAddress, (uint8_t *)&myData, sizeof(myData));

    if (result == ESP_OK) {
      Serial.println("ACK sent with success");
    } else {
      Serial.println("Error sending the ACK");
    }
  }
  if (lightOnReceived) {  
    Serial.println("hello");
    digitalWrite(motorPin, HIGH);  
    for (int thisNote = 0; thisNote < sizeof(melody) / sizeof(melody[0]); thisNote++) {
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(buzzerPin, melody[thisNote], noteDuration);
      Serial.println("note");
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(buzzerPin);
      if (buttonPressed) {
        digitalWrite(ledPin, LOW);
        digitalWrite(motorPin, LOW); // Turn off the vibration motor
        noTone(buzzerPin);
        break;
      }
    }
  }
}
