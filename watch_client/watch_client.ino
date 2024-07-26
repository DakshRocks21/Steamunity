#include <esp_now.h>
#include <WiFi.h>

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xEC, 0x64, 0xC9, 0x98, 0x7E, 0x10}; // Daksh' Board
// uint8_t broadcastAddress[] = {0x08, 0xF9, 0xE0, 0xF6, 0xE9, 0xE4}; // NOT Daksh' Board, WC Board
//uint8_t broadcastAddress[] = {0xEC, 0x64, 0xC9, 0x98, 0x79, 0x5C}; // NOT Daksh' Board, Richards Board


// ESP_TINY 
// Tested on a big board - to be modified
// Button - 12
// LED - 26
// Buzzer 25

// Define button pin
const int buttonPin = 12;
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
    tone(25, 1000, 500); // Turn on buzzer with tone
    digitalWrite(26, HIGH); // Turn on light
  }
}

void IRAM_ATTR handleButtonPress() {
  buttonPressed = true;
}

void setup() {
  // lightPin and buzzerPin
  pinMode(25, OUTPUT); // Buzzer
  pinMode(26, OUTPUT); // Light

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
    digitalWrite(26, LOW); // Turn off light
    noTone(25); // Turn off buzzer

    // Set response message
    strcpy(myData.message, "ACK");

    // Send response message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

    if (result == ESP_OK) {
      Serial.println("ACK sent with success");
    } else {
      Serial.println("Error sending the ACK");
    }
  }
  if (lightOnReceived){
     tone(25, 1000, 500);
  }
  delay(500);
}
