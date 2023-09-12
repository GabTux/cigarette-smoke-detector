
#include <MQUnifiedsensor.h>
#include <DHT.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

// GAS sensor
#define board "Arduino UNO"
#define VCC 5
#define GasPin A1
#define type "MQ-5"
#define ADC_Bit_Resolution 10
#define RatioMQ5CleanAir 6.5  //RS / R0 = 6.5 ppm 
MQUnifiedsensor MQ5(board, VCC, ADC_Bit_Resolution, GasPin, type);

// dust sensor
const int DustPin = 8;
unsigned long dustSampleTime_ms = 2000;

// DHT sensor
const int DHTPin = 2;
const int DHTTempCorrection = 2;
const int DHTHumCorrection = 0;
DHT DHTSensor(DHTPin, DHT11);

// ETHERNET
byte mac[] = {0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED};
IPAddress server(192, 168, 0, 131);
EthernetClient ethClient;
PubSubClient client(ethClient);

// inspired from here: https://github.com/knolleary/pubsubclient/blob/master/examples/mqtt_basic/mqtt_basic.ino#L38
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ArduinoClient-window";
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

void setup() {
  Serial.begin(115200);
  
  pinMode(DustPin, INPUT);

  DHTSensor.begin();
  DHTSensor.readTemperature();
  DHTSensor.readHumidity();

  MQ5.setRegressionMethod(1);
  MQ5.setA(491204); MQ5.setB(-5.826);
  /*
  Exponential regression:
  Gas    | a      | b
  H2     | 1163.8 | -3.874
  LPG    | 80.897 | -2.431
  CH4    | 177.65 | -2.56
  CO     | 491204 | -5.826
  Alcohol| 97124  | -4.918
  */
  MQ5.init();
  MQ5.setR0(10);
  
  client.setServer(server, 1883);
  Ethernet.begin(mac);
  delay(1000);
}

int mapUVIndex(float sensorRead) {
  if (sensorRead < 227)
    return 0;
  else if (227 <= sensorRead && sensorRead < 318) 
    return 1;
  else if (318 <= sensorRead && sensorRead < 408)
    return 2;
  else if (408 <= sensorRead && sensorRead < 503)
    return 3;
  else if (503 <= sensorRead && sensorRead < 606)
    return 4;    
  else if (606 <= sensorRead && sensorRead < 696)
    return 5;
  else if (696 <= sensorRead && sensorRead < 795)
    return 6;
  else if (795 <= sensorRead && sensorRead < 881)
    return 7; 
  else if (881 <= sensorRead && sensorRead < 976)
    return 8;
  else if (976 <= sensorRead && sensorRead < 1079)
    return 9;  
  else if (1079 <= sensorRead && sensorRead < 1170)
    return 10;
  else
    return 11;
}

int getUV() {
  int sensorValue;
  unsigned long sum = 0;
  const int measureCount = 10;
  for(int i=0; i < measureCount; i++)
  {
      sensorValue = analogRead(A0);
      sum = sensorValue + sum;
      delay(2);
  }
  float meanVal = sum/measureCount;
  meanVal = (meanVal * (5.0 / 1023.0))*1000;
  return mapUVIndex(meanVal);
}

float calcDust(unsigned long lowPulseOccupancy) {
  float ratio = lowPulseOccupancy/(dustSampleTime_ms*10.0);
  return 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62;
}

float getGas() {
  MQ5.update();
  return MQ5.readSensor();
}

unsigned long lastUVIndex = 0;
const unsigned int UVIndexThreshold = 60000; // 1 min

unsigned long lastGas = 0;
const unsigned int GasThreshold = 500; // 0,5 sec

unsigned long lastDHT = 0;
const unsigned int DHTThreshold = 5000; 

unsigned long dustStartTime = millis();
unsigned long lowPulseOccupancy = 0;

void sendValue(float value, String topic) {
  const int capacity = JSON_OBJECT_SIZE(1);
  StaticJsonDocument<capacity> doc;
  String sendBuffer = "";
  doc["value"] = value;
  serializeJson(doc, sendBuffer);
  Serial.println(sendBuffer);
  client.publish(topic.c_str(), sendBuffer.c_str());
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  const int capacity = JSON_OBJECT_SIZE(2);
  StaticJsonDocument<capacity> doc;
  String sendBuffer;

  unsigned long timeNow = millis();
  lowPulseOccupancy += pulseIn(DustPin, LOW);

  if (timeNow >= dustStartTime + dustSampleTime_ms)
  {
    sendValue(calcDust(lowPulseOccupancy), "/out/dust");
    lowPulseOccupancy = 0;
    dustStartTime = millis();
  }

  if (timeNow - lastUVIndex > UVIndexThreshold) {
    sendValue(getUV(), "/out/uv");
    lastUVIndex = millis();
  }

  if (timeNow - lastGas > GasThreshold) {
    sendValue(getGas(), "/out/gas");
    lastGas = millis();
  }

  if (timeNow - lastDHT > DHTThreshold) {
    const int capacity = JSON_OBJECT_SIZE(2);
    StaticJsonDocument<capacity> doc;
    String sendBuffer = "";
    doc["temp"] = DHTSensor.readTemperature() - DHTTempCorrection;
    doc["hum"] = DHTSensor.readHumidity() - DHTHumCorrection;
    serializeJson(doc, sendBuffer);
    Serial.println(sendBuffer);
    client.publish("/out/dht", sendBuffer.c_str());
    lastDHT = millis();
  }
}
