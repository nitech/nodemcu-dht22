/**
 * Example for reading temperature and humidity
 * using the DHT22 and ESP8266
 *
 * Copyright (c) 2016 Losant IoT. All rights reserved.
 * https://www.losant.com
 */


/*****************************************
 * Include Libraries
 ****************************************/
#include <ESP8266WiFi.h>
#include <ConfigManager.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTPIN 4     // 4 == 2 - see https://dziadalnfpolx.cloudfront.net/blog/wp-content/uploads/2015/09/esp8266-nodemcu-dev-kit-v3-pins.jpg
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors

DHT dht(DHTPIN, DHTTYPE);


/****************************************
 * Define Constants
 ****************************************/
 
namespace{
  const char * AP_NAME = "Sensor AP"; // Assigns your Access Point name
  const char * MQTT_SERVER = "things.ubidots.com"; 
  const char * TOKEN = "A1E-uj4jYAOzngZrIChFWUISFjJUbFJYYH"; // Assigns your Ubidots TOKEN
  const char * DEVICE_LABEL = "temptastic"; // Assigns your Device Label
  const char * VARIABLE_LABEL = "fuktighet"; // Assigns your Variable Label
  int SENSOR = A0;
}

char topic[150];
char payload[50];
String clientMac = "";
unsigned char mac[6];

struct Config {
  char name[20];
  bool enabled;
  int8 hour;
} config;

/****************************************
 * Initialize a global instance
 ****************************************/
WiFiClient espClient;
PubSubClient client(espClient);
ConfigManager configManager;


/****************************************
 * Auxiliar Functions
 ****************************************/

void callback(char* topic, byte* payload, unsigned int length){
  
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientMac.c_str(), TOKEN, NULL)) {
      Serial.println("connected");
      break;
      } else {
        configManager.reset();
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 3 seconds");
        for(uint8_t Blink=0; Blink<=3; Blink++){
          digitalWrite(LED, LOW);
          delay(500);
          digitalWrite(LED, HIGH);
          delay(500);
        }
      }
  }
}

String macToStr(const uint8_t* mac) {
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)result += ':';
  }
  return result;
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(2000);

  // Wait for serial to initialize.
  while(!Serial) { }

  Serial.println("Device Started");
  Serial.println("-------------------------------------");
  Serial.println("Running DHT!");
  Serial.println("-------------------------------------");


  // wifi
  /* Declare PINs as input/outpu */
  pinMode(SENSOR, INPUT);
  pinMode(PIN_RESET, INPUT);
  pinMode(LED, OUTPUT);

  /* Assign WiFi MAC address as MQTT client name */
  WiFi.macAddress(mac);
  clientMac += macToStr(mac);

  /* Access Point configuration */
  configManager.setAPName(AP_NAME);
  configManager.addParameter("name", config.name, 20);
  configManager.addParameter("enabled", &config.enabled);
  configManager.addParameter("hour", &config.hour);
  configManager.begin(config);

  /* Set Sets the server details */
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
  
  /* Build the topic request */
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
}

void loop() {

  // wifi
  configManager.reset();
  configManager.loop();    
  
  /* MQTT client reconnection */
  if (!client.connected()) {
      reconnect();
  }

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();


  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.println(" *C ");

  /* fuktighet */
  String humidity = String(h);
  sprintf(payload, "{\"%s\": %s}", "fuktighet", humidity.c_str());
  // Serial.println(payload);
  client.publish(topic, payload);

  /* temperatur */
  String temperature = String(t);
  sprintf(payload, "{\"%s\": %s}", "temperatur", temperature.c_str());
  // Serial.println(payload);
  client.publish(topic, payload);


  client.loop();
  delay(5000);
  
}
