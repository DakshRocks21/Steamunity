#include <esp_now.h>
#include <WiFi.h>

uint8_t doorbellAddress[] = { 0xec, 0x64, 0xc9, 0x98, 0x7e, 0x10};//mini{ 0x64, 0xe8, 0x33, 0x86, 0x4b, 0x44 }; /* Doorbell MAC address */
uint8_t watchAddress[] = { 0xec, 0x64, 0xc9, 0x98, 0x79, 0x5c}; // mini{ 0x64, 0xe8, 0x33, 0x85, 0xfe, 0x00 };  /* Watch MAC address */

typedef struct struct_message {
  char message[32];
} struct_message;

struct_message myData;
struct_message receivedData;

esp_now_peer_info_t peerInfoDoorbell;
esp_now_peer_info_t peerInfoWatch;

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
    esp_err_t result = esp_now_send(watchAddress, (uint8_t *)&receivedData, sizeof(receivedData));
    if (result == ESP_OK) {
      strcpy(myData.message, "ACK_0");
    } else {
      strcpy(myData.message, "FAILED");
    }
    esp_now_send(doorbellAddress, (uint8_t *)&myData, sizeof(myData));
  } else if (strcmp(receivedData.message, "ACK") == 0) {
    strcpy(myData.message, "ACK_2");
    esp_now_send(watchAddress, (uint8_t *)&myData, sizeof(myData));
    
    strcpy(myData.message, "ACK_1");
    esp_now_send(doorbellAddress, (uint8_t *)&myData, sizeof(myData));
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  memcpy(peerInfoDoorbell.peer_addr, doorbellAddress, 6);
  peerInfoDoorbell.channel = 0;
  peerInfoDoorbell.encrypt = false;
  if (esp_now_add_peer(&peerInfoDoorbell) != ESP_OK) {
    Serial.println("Failed to add peer: Doorbell");
    return;
  }

  memcpy(peerInfoWatch.peer_addr, watchAddress, 6);
  peerInfoWatch.channel = 0;
  peerInfoWatch.encrypt = false;
  if (esp_now_add_peer(&peerInfoWatch) != ESP_OK) {
    Serial.println("Failed to add peer: Watch");
    return;
  }
}

void loop() {
  // Nothing to do here, everything is handled in the callbacks
}
