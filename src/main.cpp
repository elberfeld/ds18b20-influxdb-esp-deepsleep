#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <InfluxDb.h>

// -------- Settings --------

#define INFLUXDB_HOST ""
#define INFLUXDB_DB ""
#define INFLUXDB_USER ""
#define INFLUXDB_PASS ""
#define WIFI_SSID ""
#define WIFI_PASS ""

#define ESP_LED_PIN D4 // Status LED of wemos d1 mini lite board, An solange aktiv, low=an

#define ONE_WIRE_BUS D6 // Data pin of ds18b20

uint32_t sleep_time = 300; //in s, 470R zwischen D0 und RST!

// --------  --------

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
Influxdb influx(INFLUXDB_HOST);

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
	
	Serial.begin(9600);
	Serial.println("### Start ###");
	
	Serial.println("Init Sensor...");
	pinMode(D5, OUTPUT);  // gnd for sensor
	digitalWrite(D5, LOW);
	pinMode(D7, OUTPUT);  // vcc for sensor
	digitalWrite(D7, HIGH);
	delay(100);
	sensors.begin();
	sensors.getAddress(sensorDeviceAddress, 0);
	sensors.setResolution(sensorDeviceAddress, SENSOR_RESOLUTION);

  	if(WiFi.status() != WL_CONNECTED)wifi_connect();
	if(WiFi.status() != WL_CONNECTED)wifi_connect();
	if(WiFi.status() != WL_CONNECTED)wifi_connect();
	if(WiFi.status() != WL_CONNECTED)wifi_connect();
	if(WiFi.status() != WL_CONNECTED)wifi_connect();
	if(WiFi.status() != WL_CONNECTED){
		Serial.println("WLAN nicht verbindbar, sleep 5 min.");
		go_to_sleep(300); //0=forever
	}

	influx.setDbAuth(INFLUXDB_DB, INFLUXDB_USER, INFLUXDB_PASS);

	Serial.println("Setup done");
}


int loopCount = 0;

void loop() {
  // loopCount++;
  
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  float temp = sensors.getTempCByIndex(0);
  // float temp = 15.0;
  Serial.println("DONE");
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  Serial.print("Temperature for the device 1 (index 0) is: ");
  Serial.println(temp);  

  InfluxData row("temperatur");
  row.addTag("kunde", "ff");
  row.addTag("standort", "bueroms");
  row.addTag("art", "iot-temperatur");
  row.addTag("objektname", "chefetage1");
  row.addValue("temperatur", temp);

  influx.write(row);

  // delay(sleep_time);
  go_to_sleep(sleep_time); //0=forever

}

