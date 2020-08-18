#include <WiFi.h>
#include <Wire.h>
#include <SparkFunHTU21D.h>
#include <PubSubClient.h>
#include "device_config.h"

const long uS_TO_S_FACTOR = 1000000;

const String TOPIC_TEMP =  TOPIC_BASE + "/" + CLIENT_NAME + "/" + "temperature";
const String TOPIC_HUMD =  TOPIC_BASE + "/" + CLIENT_NAME + "/" + "humidity";

HTU21D sensor;
WiFiClient espClient;
PubSubClient client(espClient);

void wifiConnect()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("connected");
  Serial.print("IP address is: ");
  Serial.println(WiFi.localIP());
}

void mqttConnect()
{
  while (!client.connected()) {
    Serial.print("Connecting to MQTT broker...");
    
    if (client.connect(CLIENT_NAME.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void publishSensorValues()
{
  // Read sensor values
  sensor.begin();
  float humd = sensor.readHumidity();
  float temp = sensor.readTemperature();

  Serial.print("Publishing: ");
  
  if(temp >= -100.0 && temp <= 100.0)
  {
    // some basic validation
    Serial.print(temp, 1);
    Serial.print("C ");

    // Publish to MQTT
    client.publish(TOPIC_TEMP.c_str(), String(temp).c_str());
  }

  if(humd >= 0.0 && humd <= 100.0)
  {
    // some basic validation
    Serial.print(humd, 1);
    Serial.print("%");
    Serial.println();
  
    // Publish to MQTT
    client.publish(TOPIC_HUMD.c_str(), String(humd).c_str()); 
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);
  esp_sleep_enable_timer_wakeup(INTERVAL * uS_TO_S_FACTOR);

  // Set up some pins to supply sensor. Current low enough to not exceed max draw on GPIO
  int h_pin = 19;
  int l_pin = 23;
  pinMode(h_pin, OUTPUT);
  pinMode(l_pin, OUTPUT);
  digitalWrite(h_pin, HIGH);
  digitalWrite(l_pin, LOW);

  // Connect to Wifi network
  wifiConnect();
  // Connect to MQTT broker
  client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  mqttConnect();

  publishSensorValues();
  
  Serial.println("Going back to sleep now...");
  delay(1000);
  Serial.flush(); 
  esp_deep_sleep_start();
}

void loop()
{
  // nothing to loop
}
