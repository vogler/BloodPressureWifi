// WiFi
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
WiFiClient wifi;
// MQTT
#include <PubSubClient.h>
PubSubClient mqtt(wifi);
// json
char buf[200];
#define json(s, ...) (sprintf(buf, "{ " s " }", __VA_ARGS__), buf)

void setup_wifi() {
  delay(5);
  Serial.printf("Connecting to AP %s", WIFI_SSID);
  const unsigned long start_time = millis();
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  for (int i = 0; WiFi.waitForConnectResult() != WL_CONNECTED && i < 10; i++) {
    #ifdef ESP32
      WiFi.begin(WIFI_SSID, WIFI_PASS); // for ESP32 also had to be moved inside the loop, otherwise only worked on every second boot, https://github.com/espressif/arduino-esp32/issues/2501#issuecomment-548484502
    #endif
    delay(200);
    Serial.print(".");
  }
  const float connect_time = (millis() - start_time) / 1000.;
  Serial.println();
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("Failed to connect to Wifi in %.3f seconds. Going to restart!", connect_time);
    ESP.restart();
  }
  Serial.printf("Connected in %.3f seconds. IP address: ", connect_time);
  Serial.println(WiFi.localIP());
}

void mqtt_connect(){
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (mqtt.connect(clientId.c_str())) {
      Serial.printf("connected as %s to mqtt://%s\n", clientId.c_str(), MQTT_SERVER);
      // mqtt.publish(MQTT_TOPIC "/status", json("\"millis\": %lu, \"event\": \"connect\", \"clientId\": \"%s\"", millis(), clientId.c_str())); // TODO millis() is really just the ms, not full unix timestamp!
      // mqtt.subscribe("");
    } else {
      Serial.printf("failed, rc=%d. retry in 5s.\n", mqtt.state());
      delay(5000);
    }
  }
}

void setup_mqtt() {
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  // mqtt.setCallback(mqtt_callback);
  randomSeed(micros());
  mqtt_connect();
}
