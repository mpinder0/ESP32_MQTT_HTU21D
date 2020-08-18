// Wifi SSID and Password
const char* ssid = "Wifi_Name";
const char* password =  "Wifi_Password";

// IP address of MQTT broker to publish to
const uint8_t mqtt_server[] = {192, 168, 0, 99};
// Publish interval - how often to send values to MQTT broker in milliseconds
const long INTERVAL = 30000;

// MQTT topic information
const String CLIENT_NAME =  "ESP32_kitchen";
const String TOPIC_BASE =  "home";
// resulting MQTT topic will be like <TOPIC_BASE>/<CLIENT_NAME>/<sensor> so e.g. home/ESP32_kitchen/temperature
