// Wifi SSID and Password
const char* WIFI_SSID = "Wifi_Name";
const char* WIFI_PASSWORD =  "Wifi_Password";

// IP address of MQTT broker to publish to
const uint8_t MQTT_SERVER_IP[] = {192, 168, 0, 99};
const int MQTT_SERVER_PORT = 1883;

// MQTT topic information
const String CLIENT_NAME =  "ESP32_kitchen";
const String TOPIC_BASE =  "home";
// resulting MQTT topic will be like <TOPIC_BASE>/<CLIENT_NAME>/<sensor> so e.g. home/ESP32_kitchen/temperature

// Publish interval - how often to send values to MQTT broker in seconds
const int INTERVAL = 30;
