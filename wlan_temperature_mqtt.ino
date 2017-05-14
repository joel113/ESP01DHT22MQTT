#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DHT.h"

extern "C" {
  #include "user_interface.h"
}

#define DHTTYPE DHT22

const char* ssid = "";
const char* password = "";
char* wifiHostName = "";

const char* mqtt_server = "";

WiFiClient espClient;
PubSubClient client(espClient);

const int DHTPin = 2;
DHT dht(DHTPin, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  wifi_station_set_auto_connect(true);
  wifi_station_set_hostname(wifiHostName);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  client.setServer(mqtt_server, 1883);
  delay(10000);
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  delay(5000);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  else {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& object = jsonBuffer.createObject();
    object["temperature"] = t;
    object["humidity"] = h;
    
    object.prettyPrintTo(Serial);
  
    String json_value;
    object.printTo(json_value);
  
    char mqtt_value[200];
    json_value.toCharArray(mqtt_value, 200);
    
    client.publish("/joel-weatherman/sensor", mqtt_value, true);
  }
}
