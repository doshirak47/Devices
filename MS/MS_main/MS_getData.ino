#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SGP30.h>

#define BME_SCK D6
#define BME_MISO D5
#define BME_MOSI D7
#define BME_CS D3

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI
Adafruit_SGP30 sgp;
//данные метеостанции
float Temp = 0;
float Humidity = 0;
float CO2 = 0;
float Pressure = 0;

//топики
String top1 = device_name+"/temp";
String top2 = device_name+"/press";
String top3 = device_name+"/hum";
String top4 = device_name+"/CO2";


void call(String& top, String& val){//if(top == device_name + "/"){};
  if(top == device_name + "/request"){
    if(val == "get"){
      send_Data();
    }
  }
}

void setup_topic(){//mqtt_client.subscribe(String(id)+"/");
  mqtt_client.subscribe(String(id)+"/request");
}

void setup_external(){
   if (! sgp.begin()){
    Serial.println("Sensor not found :(");
    while (1);
  }
  bme.begin();
}

uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}

void get_Data(){//получаем данные с датчиков
  Temp = bme.readTemperature();
  Pressure = (bme.readPressure()/ 100.0F);
  Humidity = bme.readHumidity();
  Serial.print("T = ");
  Serial.println(Temp);
  Serial.print("P = ");
  Serial.println(Pressure);
  Serial.print("H = ");
  Serial.println(Humidity);
  sgp.setHumidity(getAbsoluteHumidity(Temp, Humidity));
  if (! sgp.IAQmeasure()) {
    Serial.println("Measurement failed");
    return;
  }
  CO2=sgp.eCO2;
  if (! sgp.IAQmeasureRaw()) {
    Serial.println("Raw Measurement failed");
    return;
  }
  Serial.println();
  delay(1000);
}
void send_Data(){//отправляем на mqtt (по запросу)
  mqtt_client.publish(top1,String(Temp));
  mqtt_client.publish(top2,String(Pressure));
  mqtt_client.publish(top3,String(Humidity));
  mqtt_client.publish(top4,String(CO2));
}
