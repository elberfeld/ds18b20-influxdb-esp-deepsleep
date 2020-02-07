#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// -------- Settings --------

#define WIFI_SSID ""
#define WIFI_PASS ""

#define MQTT_HOST ""
#define MQTT_PORT 1883
#define MQTT_ID ""
#define MQTT_TOPIC "temp"

uint32_t sleep_time = 30;  //in s

// -------- Settings --------

#define ESP_LED_PIN D4  // Status LED of wemos d1 mini lite board, An solange aktiv, low=an

#define ONE_WIRE_BUS D4  // Data pin of ds18b20

#define DS18B20_POWERPIN D2  // io as vcc, comment out to disable, use 100n cap on sensor when doing this!


// 9 bits: increments of 0.5C, 93.75ms to measure temperature;
// 10 bits: increments of 0.25C, 187.5ms to measure temperature;
// 11 bits: increments of 0.125C, 375ms to measure temperature;
// 12 bits: increments of 0.0625C, 750ms to measure temperature.
#define SENSOR_RESOLUTION 12


// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
DeviceAddress sensorDeviceAddress;

ESP8266WiFiMulti WiFiMulti;
WiFiClient wifiClient;
PubSubClient mqttClient(MQTT_HOST, MQTT_PORT, wifiClient);


void wifi_connect(){
	Serial.print("WLAN: connecting to \"" + (String)WIFI_SSID + "\"");
	WiFi.begin(WIFI_SSID, WIFI_PASS);

	uint16_t tries=0;
	while (tries < 30 && WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print('.');
		tries++;
	}
	
	Serial.println("");
	
	if(WiFi.status() != WL_CONNECTED){
		Serial.println("WLAN: Fehler: keine WLAN Verbindung moeglich");
	}else{
		Serial.print("WLAN: connected, IP address: ");
		Serial.println(WiFi.localIP());
	}
}

void wifi_disconnect(){
	Serial.println("WLAN: disconnected");
	WiFi.disconnect();
}

void go_to_sleep(uint32_t seconds){
	wifi_disconnect();
	if(seconds == 0){
		Serial.println("Going to Sleep FOREVER!!!");
	}else{
		Serial.println("Going to Sleep for " + (String)seconds + 's');
	}
	Serial.println("");
	//`ESP.deepSleep(microseconds, mode)` will put the chip into deep sleep. `mode` is one of `WAKE_RF_DEFAULT`, `WAKE_RFCAL`, `WAKE_NO_RFCAL`, `WAKE_RF_DISABLED`. (GPIO16 needs to be tied to RST to wake from deepSleep.)
	// digitalWrite(ESP_LED_PIN, HIGH); //statusled aus
	ESP.deepSleep(seconds*1000000, WAKE_RF_DEFAULT);
	//delay(seconds*1000);
}



void setup() {
	pinMode(ESP_LED_PIN, OUTPUT);
	digitalWrite(ESP_LED_PIN, LOW); //statusled an

	#ifdef DS18B20_POWERPIN
	pinMode(DS18B20_POWERPIN, OUTPUT);
	digitalWrite(DS18B20_POWERPIN, HIGH); //artificial vcc
	#endif

	Serial.begin(9600);
	Serial.println("");
	Serial.println("### Start ###");
	
  	if(WiFi.status() != WL_CONNECTED)wifi_connect();
	if(WiFi.status() != WL_CONNECTED)wifi_connect();
	if(WiFi.status() != WL_CONNECTED)wifi_connect();
	if(WiFi.status() != WL_CONNECTED)wifi_connect();
	if(WiFi.status() != WL_CONNECTED)wifi_connect();
	if(WiFi.status() != WL_CONNECTED){
		Serial.println("WLAN nicht verbindbar, sleep 5 min.");
		go_to_sleep(300); //0=forever
	}


	Serial.println("Init Sensor...");
	delay(100);
	sensors.begin();
	sensors.getAddress(sensorDeviceAddress, 0);
	sensors.setResolution(sensorDeviceAddress, SENSOR_RESOLUTION);

	Serial.print("Requesting temperatures...");
	sensors.requestTemperatures(); // Send the command to get temperatures
	float temp = sensors.getTempCByIndex(0);
	Serial.println(" DONE");
	Serial.print("Temperature for the device 1 (index 0) is: ");
	Serial.println(temp);  

    char topic[strlen(MQTT_TOPIC) + strlen(MQTT_ID) + 2];
    sprintf(topic, "%s/%s", MQTT_TOPIC, MQTT_ID);

    char payload[10]; 
    sprintf(payload, "%2.2f", temp);

    Serial.print("MQTT Topic: ");
    Serial.print(topic);
    Serial.print(" :: Payload: ");
    Serial.print(payload);
    Serial.println();

	if (mqttClient.connect(MQTT_ID)) {
		Serial.println("MQTT connected");
	} else {
		Serial.println("MQTT connect failed");
		Serial.println(mqttClient.state());
	}

	if (mqttClient.publish(topic, payload)) {
		Serial.println("MQTT published");
	} else {
		Serial.println("MQTT publish failed, state="+mqttClient.state());
		Serial.println(mqttClient.state());
	}	

	mqttClient.disconnect();
	Serial.println("MQTT disconnected");

	// delay(sleep_time);
	go_to_sleep(sleep_time); //0=forever
}

void loop() {}
