#include <esp_now.h>
#include <WiFi.h>

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress1[] = {0xEC, 0x64, 0xC9, 0x98, 0x79, 0x5C}; // RICHARD BOARD MAC address of Board 2 RICHARD
//uint8_t broadcastAddress2[] = {0x08, 0xF9, 0xE0, 0xF6, 0xE9, 0xE4}; // MAC address of Board 3 OTHER BOARD (RED LIGHT); 08:f9:e0:f6:e9:e4

// ESP Tiny Address: 24:58:7c:ae:c6:a4
//uint8_t broadcastAddress1[] = {0x24, 0x58, 0x7c, 0xae, 0xc6, 0xa4};

// Doorbell
// BUTTON 12 and Ground
// Light 25
// Screen - TBC (when it comes)

// Define button pin
const int buttonPin = 26;
bool buttonPressed = false;
unsigned long lastSendTime = 0; // Timestamp of the last message sent
const unsigned long sendTimeout = 0; // 15 seconds timeout
const int toAccept = 25;
const int accepted = 33;

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

  digitalWrite(accepted, HIGH);
  digitalWrite(toAccept, LOW);
}

void IRAM_ATTR handleButtonPress() {
  buttonPressed = true;
}

void setup() {
  //lightPins
  pinMode(toAccept, OUTPUT);
  pinMode(accepted, OUTPUT);

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

  // Register peers
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer 1");
    return;
  }

  // memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
  // peerInfo.channel = 0;
  // peerInfo.encrypt = false;
  // if (esp_now_add_peer(&peerInfo) != ESP_OK) {
  //   Serial.println("Failed to add peer 2");
  //   return;
  // }

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
      esp_err_t result1 = esp_now_send(broadcastAddress1, (uint8_t *) &myData, sizeof(myData));
      //esp_err_t result2 = esp_now_send(broadcastAddress2, (uint8_t *) &myData, sizeof(myData));

      if (result1 == ESP_OK){// && result2 == ESP_OK) {
        Serial.println("Sent with success to peers");
      } else {
        Serial.println("Error sending the data");
      }
    } else {
      Serial.println("Timeout in effect. Please wait before sending again.");
    }
  }
}
