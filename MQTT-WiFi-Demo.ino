#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiMulti.h>
#include "esp_wifi.h"
#include "esp_pm.h"

WiFiMulti wifiMulti;

const char* mqttServer = "MQTT SERVER IP ADDRESS";
const int mqttPort = 1883; //enter correct MQTT port here
const char* mqttTopicPrefix = "button/test";  //change this to suit your needs

WiFiClient espClient;
PubSubClient client(espClient);

const int buttonPins[] = {32, 33, 25, 26, 27, 14, 12, 13}; 
const int numButtons = sizeof(buttonPins) / sizeof(buttonPins[0]);

void IRAM_ATTR wakeUp();  

void setup() {
  Serial.begin(115200);

  wifiMulti.addAP("Your first SSID", "Your WiFi key");
  wifiMulti.addAP("Your second SSID", "Your WiFi key");

  while (wifiMulti.run() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  
  // Enable Light Sleep Mode
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

  client.setServer(mqttServer, mqttPort);

  for (int i = 0; i < numButtons; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    attachInterrupt(buttonPins[i], wakeUp, FALLING);
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  for (int i = 0; i < numButtons; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
        delay(50);  // Simple debounce. Adjust as necessary.
        if (digitalRead(buttonPins[i]) == LOW) { // Check again after delay
            processButtonPress(i);
            break; // Exit after processing one button to prevent multiple simultaneous presses.
        }
    }
  }
}

void processButtonPress(int buttonIndex) {
  String message = "Button " + String(buttonIndex + 1);
  String topic = mqttTopicPrefix + String(buttonIndex + 1);
  client.publish(topic.c_str(), message.c_str());
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("Connection failed, retrying...");
      delay(2000);
    }
  }
}

void IRAM_ATTR wakeUp() {
    // Do nothing, just act as a wake-up trigger
}
