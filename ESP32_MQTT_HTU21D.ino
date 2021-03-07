#include <WiFi.h>
#include <Wire.h>
#include <SparkFunHTU21D.h>
#include <PubSubClient.h>
#include "device_config.h"

const bool WIFI_DEBUG = false;
const int MAX_CONNECT_ATTEMPTS = 3;
const int CONNECT_ATTEMPT_DURATION = 5; // seconds(ish)
const long uS_TO_S_FACTOR = 1000000;

const int LED_PIN = 2;
const int xshort_duration = 5;
const int short_duration = 100;
const int long_duration = 1000;

const String TOPIC_TEMP =  TOPIC_BASE + "/" + CLIENT_NAME + "/" + "temperature";
const String TOPIC_HUMD =  TOPIC_BASE + "/" + CLIENT_NAME + "/" + "humidity";

HTU21D sensor;
WiFiClient espClient;
PubSubClient client(espClient);

void debugFlasher(int n)
{
  if(n < 1)
  {
    // 0 or less, do one long flash
    digitalWrite(LED_PIN, HIGH);
    delay(long_duration);
    digitalWrite(LED_PIN, LOW);
  }
  else if(n > 10)
  {
    // greater than 10, do two long flashes
    digitalWrite(LED_PIN, HIGH);
    delay(long_duration);
    digitalWrite(LED_PIN, LOW);
    delay(long_duration);
    digitalWrite(LED_PIN, HIGH);
    delay(long_duration);
    digitalWrite(LED_PIN, LOW);
  }
  else
  {
    // otherwise short-flash the LED the specified number of times
    for(int i=0; i<n; i++)
    {
      digitalWrite(LED_PIN, HIGH);
      delay(short_duration);
      digitalWrite(LED_PIN, LOW);
      delay(short_duration);
    }
  }
  // extra long delay at the end to separate flash groups
  delay(long_duration * 5);
}

bool wifiConnect()
{
  int attempts = 0;
  int wifi_status = WL_IDLE_STATUS;
  
  do
  {
    Serial.print("Connecting to WiFi..");
    attempts++;
    wifi_status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    if(WIFI_DEBUG == true)
    {
      Serial.println(wifi_status);
    }

    int attempt_checked = 0;
    while(wifi_status != WL_CONNECTED && attempt_checked < CONNECT_ATTEMPT_DURATION)
    {
      attempt_checked++;
      wifi_status = WiFi.status();
      
      if(WIFI_DEBUG == true)
      {
        Serial.println(wifi_status);
        digitalWrite(LED_PIN, HIGH);
        delay(xshort_duration);
        digitalWrite(LED_PIN, LOW);
        delay(1000 - xshort_duration);
      }
      else
      {
        Serial.print(".");
        delay(1000);
      }
    }

    // At the end of each attempt, when enabled, flash the last status code on the LED for debugging purposes
    if(WIFI_DEBUG == true)
    {
      debugFlasher(wifi_status);
    }
  }
  while (wifi_status != WL_CONNECTED && attempts < MAX_CONNECT_ATTEMPTS);

  if(wifi_status == WL_CONNECTED)
  {
    Serial.println("connected");
    Serial.print("IP address is: ");
    Serial.println(WiFi.localIP());
    return true;
  }
  else
  {
    Serial.println("Exceeded max retries. Failed to connect.");
    return false;
  }
}

bool mqttConnect()
{
  Serial.print("Connecting to MQTT broker...");
  
  client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  bool mqtt_success = client.connect(CLIENT_NAME.c_str());
  if(mqtt_success == true)
  {
    Serial.println("connected");
  }
  else
  {
    Serial.println("failed :(");
  }
  return mqtt_success;
}

void publishSensorValues()
{
  // Read sensor values
  sensor.begin();
  float humd = sensor.readHumidity();
  float temp = sensor.readTemperature();

  Serial.println("Publishing: ");

      
  Serial.print(temp, 1);
  Serial.print("C ");
  // some basic validation
  if(temp >= -100.0 && temp <= 100.0)
  {
    // Publish to MQTT
    client.publish(TOPIC_TEMP.c_str(), String(temp).c_str());
  }
  else
  {
    Serial.print(" validation failed");
  }
  Serial.println();

  Serial.print(humd, 1);
  Serial.print("%");
  // some basic validation
  if(humd >= 0.0 && humd <= 100.0)
  {
    // Publish to MQTT
    client.publish(TOPIC_HUMD.c_str(), String(humd).c_str()); 
  }
  else
  {
    Serial.print(" validation failed");
  }
  Serial.println();
  
}

void setup()
{
  Serial.begin(115200);
  delay(1000);
  
  // Info LED setup
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  
  // Set up some pins to supply sensor. Current low enough to not exceed max draw on GPIO
  int h_pin = 19;
  int l_pin = 23;
  pinMode(h_pin, OUTPUT);
  pinMode(l_pin, OUTPUT);
  digitalWrite(h_pin, HIGH);
  digitalWrite(l_pin, LOW);

  /*
  esp_sleep_enable_timer_wakeup(INTERVAL * uS_TO_S_FACTOR);

  // Connect to Wifi network
  bool success = false;
  success = wifiConnect();
  if(success == true)
  {
    // Connect to MQTT broker
    if(mqttConnect() == true)    
    {
      publishSensorValues();
    }
  }
  
  Serial.print("Going back to sleep now for ");
  Serial.print(INTERVAL);
  Serial.println(" seconds...");
  delay(1000);
  Serial.flush(); 
  esp_deep_sleep_start();
  */
}


void loop()
{
  bool wifi_connected = false;
  bool mqtt_connected = false;
  
  // Connect to Wifi network
  if(WiFi.status() == WL_CONNECTED)
  {
    wifi_connected = true;
  }
  else
  {
    wifi_connected = wifiConnect();
  }
  
  if(wifi_connected == true)
  {
    // Connect to MQTT broker
    if(client.state() == MQTT_CONNECTED)
    {
      mqtt_connected = true;
    }
    else 
    {
      mqtt_connected = mqttConnect();
    }
    
    if(mqtt_connected == true)    
    {
      publishSensorValues();
    }
  }
  Serial.print("Sleeping now...");
  delay(INTERVAL * 1000);
  
}
