#include "WiFi.h"
#include <Wire.h>
#include "SparkFunHTU21D.h"
#include <PubSubClient.h>
#include "device_config.h"

const String TOPIC_TEMP =  TOPIC_BASE + "/" + CLIENT_NAME + "/" + "temperature";
const String TOPIC_HUMD =  TOPIC_BASE + "/" + CLIENT_NAME + "/" + "humidity";

HTU21D myHumidity;
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsgTime = 0;
char buffer[10];

void wifi_setup()
{
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.print("Connected! Local IP address is: ");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = CLIENT_NAME + "-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      //client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  randomSeed(micros());

  // Set up some pins to supply sensor. Current low enough to not exceed max draw on GPIO
  int h_pin = 19;
  int l_pin = 23;
  pinMode(h_pin, OUTPUT);
  pinMode(l_pin, OUTPUT);
  digitalWrite(h_pin, HIGH);
  digitalWrite(l_pin, LOW);

  wifi_setup();
  client.setServer(mqtt_server, 1883);
  //client.setCallback(callback);
  
  myHumidity.begin();
}

void loop()
{
  float humd = myHumidity.readHumidity();
  float temp = myHumidity.readTemperature();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsgTime > INTERVAL) {
    lastMsgTime = now;
    Serial.print("Publishing: ");  
    Serial.print(temp, 1);
    Serial.print("C ");
    Serial.print(humd, 1);
    Serial.print("%");
    Serial.println();
    
    client.publish(TOPIC_TEMP.c_str(), String(temp).c_str());
    client.publish(TOPIC_HUMD.c_str(), String(humd).c_str());
  }
}
