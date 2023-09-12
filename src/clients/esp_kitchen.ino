
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

// wifi
const char* wifiSSID = "REDACTED";
const char* wifiPass = "REDACTED";
const char* mqtt_server = "192.168.0.131";
WiFiClient espClient;
PubSubClient client(espClient);

// dallas temperature
const int onewirePIN = D4;
OneWire oneWire(onewirePIN);
DallasTemperature dsSensors(&oneWire);

// tsl light
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_LOW, 12345);


// inspired by library example: https://github.com/knolleary/pubsubclient/blob/master/examples/mqtt_esp8266/mqtt_esp8266.ino
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifiSSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID, wifiPass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-kitchen";
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

float getTemp() {
  dsSensors.requestTemperatures();
  return dsSensors.getTempCByIndex(0);
}

float getLight() {
  sensors_event_t event;
  tsl.getEvent(&event);
   return event.light;
}

void sendValue(float value, String topic) {
  const int capacity = JSON_OBJECT_SIZE(1);
  StaticJsonDocument<capacity> doc;
  String sendBuffer = "";
  doc["value"] = value;
  serializeJson(doc, sendBuffer);
  Serial.println(sendBuffer);
  client.publish(topic.c_str(), sendBuffer.c_str());
}



void setup(void) {
  // debugging serial
  Serial.begin(115200);
  while (!Serial);
  
  // temperature
  dsSensors.begin();

  // light
  if(!tsl.begin())
  {
    Serial.print("No TSL2561 detected");
    while(1);
  }
  tsl.enableAutoRange(true);
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  
  
  // wifi
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}


unsigned long lastTemp = 0;
const int tempThreshold = 10000; // 10 secs
unsigned long lastLight = 0;
const int lightThreshold = 5000; // 5 secs

void loop(void) {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long timeNow = millis();
  if (timeNow - lastTemp > tempThreshold) {
    sendValue(getTemp(), "/kitchen/temperature");
    lastTemp = millis();
  }

  if (timeNow - lastLight > lightThreshold) {
    sendValue(getLight(), "/kitchen/light");
    lastLight = millis();
  }
}

