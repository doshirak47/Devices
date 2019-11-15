#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SGP30.h>
#include <ESP8266WiFi.h>
#include <epd.h>
#include <TimeLib.h>                      // Библиотека для работы с временем
#include <ArduinoJson.h>                  // Библиотека для разбора JSON

String regionID = "2";                  // Код региона по Yandex для выбора часового пояса https://tech.yandex.ru/xml/doc/dg/reference/regions-docpage/

String SunriseTime, SunsetTime, Temperature;
char icon[20];

const int wake_up = 5;
const int reset = 4;

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
String Time = "";
String Weather = "";

//топики
String top1 = device_name+"/temp";
String top2 = device_name+"/press";
String top3 = device_name+"/hum";
String top4 = device_name+"/CO2";
String top5 = device_name+"/Time";
String top6 = device_name+"/Weather";


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
  epd_init(wake_up,reset);
  epd_wakeup(wake_up);
  epd_clear();
  epd_set_memory(MEM_NAND);
  epd_set_en_font(ASCII48);
}

uint32_t getAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}

bool TimeAndWeather () {                                                    // Функция синхронизации времени работы программы с реальным временем и получения информации о погоде
  if (WiFi_client2.connect("yandex.com",443)) {                                   // Если удаётся установить соединение с указанным хостом (Порт 443 для https)
    WiFi_client2.println("GET /time/sync.json?geo=" + regionID + " HTTP/1.1\r\nHost: yandex.com\r\nConnection: close\r\n\r\n"); // Отправляем параметры запроса
    delay(200);                                                             // Даём серверу время, чтобы обработать запрос
    char endOfHeaders[] = "\r\n\r\n";                                       // Системные заголовки ответа сервера отделяются от остального содержимого двойным переводом строки
    if (!WiFi_client2.find(endOfHeaders)) {                                       // Отбрасываем системные заголовки ответа сервера
      Serial.println("Invalid response");                                   // Если ответ сервера не содержит системных заголовков, значит что-то пошло не так
      return false;                                                         // и пора прекращать всё это дело
    }
    const size_t capacity = 750;                                            // Эта константа определяет размер буфера под содержимое JSON (https://arduinojson.org/v5/assistant/)
    DynamicJsonBuffer jsonBuffer(capacity);                                 // Инициализируем буфер под JSON
    
    JsonObject& root = jsonBuffer.parseObject(WiFi_client2);                      // Парсим JSON-модержимое ответа сервера
    WiFi_client2.stop();                                                          // Разрываем соединение с сервером

    String StringCurrentTime = root["time"].as<String>().substring(0,10);   // Достаём значение реального текущего времени из JSON и отбрасываем от него миллисекунды
    String StringOffset =  root["clocks"][regionID]["offset"].as<String>(); // Достаём значение смещения времени по часовому поясу (в миллисекундах)
    SunriseTime =  root["clocks"][regionID]["sunrise"].as<String>();        // Достаём время восхода - Третий уровень вложенности пары ключ/значение clocks -> значение RegionID -> sunrise 
    SunsetTime =  root["clocks"][regionID]["sunset"].as<String>();          // Достаём время заката - Третий уровень вложенности пары ключ/значение clocks -> значение RegionID -> sunset
    Temperature =  root["clocks"][regionID]["weather"]["temp"].as<String>();// Достаём время заката - Четвёртый уровень вложенности пары ключ/значение clocks -> значение RegionID -> weather -> temp
    strcpy(icon, root["clocks"][regionID]["weather"]["icon"].as<String>().c_str());       // Достаём иконку - Четвёртый уровень вложенности пары ключ/значение clocks -> значение RegionID -> weather -> icon

    jsonBuffer.clear();                                                     // Очищаем буфер парсера JSON
    
    unsigned long CurrentTime = StringToULong(StringCurrentTime);           // Переводим значение реального времени в секундах, считанное с Яндекса, из String в unsigned long
    unsigned long Offset = StringToULong(StringOffset) / 1000;              // Переводим значение смещения времени по часовому поясу, считанное с Яндекса, из String в unsigned long и переводим его в секунды
    setTime(CurrentTime + Offset);                                          // Синхронизируем время
    
    return true;
  }
}

unsigned long StringToULong(String Str) {                     // Эта функция преобразует String в unsigned long
  unsigned long ULong = 0;
  for (int i = 0; i < Str.length(); i++) {                    // В цикле посимвольно переводим строку в unsigned long
     char c = Str.charAt(i);
     if (c < '0' || c > '9') break;
     ULong *= 10;
     ULong += (c - '0');
  }
  return ULong;
}

String digitalClock(){                                   // Эта функция выводит дату и время на монитор серийного порта
  return leadNull(hour())+":"+leadNull(minute());
}

String leadNull(int digits){                                    // Функция добавляет ведущий ноль
  String out = "";
  if(digits < 10)
    out += "0";                                               
  return out + String(digits);
}

String WeatherDisplay(){  
  char * out = strtok(icon,"-");        // Выделяем первую часть из строки до символа '-'
  while (out != NULL) {                 // Выделяем последующие части строки в цикле, пока значение out не станет нулевым (пустым)
      if (String(out) == "skc")         // Перебираем в условиях все возможные варианты, зашифрованные в названии иконки
        return(" Yasno  ");
      else if (String(out) == "ovc")
        return("Pasmurno");
      else if (String(out) == "bkn")
        return("Oblachno");
      else if (String(out) == "ra")
        return(" Dozhd' ");
      else if (String(out) == "ts")
        return(" Groza  ");
      else if (String(out) == "sn")
        return("  Sneg  ");
      else if (String(out) == "bl")
        return(" Metel' ");
      else if (String(out) == "fg")
        return(" Tuman  ");
      else if (String(out) == "n")
        return("\nTemnoe vremya sutok");
      else if (String(out) == "d")
        return("\nSvetloe vremya sutok");
      
      out = strtok(NULL,"-");              // Выделяем очередную часть
   }
}


void draw_text(String t, String h, String c, String p, String ti, String w)
{
  t = "Temperature = " + t;
  char buf1[t.length()+1];
  t.toCharArray(buf1,t.length()+1);
  epd_disp_string(buf1, 50, 50);

  h = "Humidity = " + h;
  char buf2[h.length()+1];
  h.toCharArray(buf2,h.length()+1);
  epd_disp_string(buf2, 500, 50);

  c = "CO2(ppm) = " + c;
  char buf3[c.length()+1];
  c.toCharArray(buf3,c.length()+1);
  epd_disp_string(buf3, 50, 500);

  p = "Pressure = " + p;
  char buf4[p.length()+1];
  p.toCharArray(buf4,p.length()+1);
  epd_disp_string(buf4, 500, 500);
  
  
  char buf5[ti.length()+1];
  ti.toCharArray(buf5,ti.length()+1);
  epd_disp_string(buf5, 360, 90);
  
  char buf6[w.length()+1];
  w.toCharArray(buf6,w.length()+1);
  epd_disp_string(buf6, 340, 130);

  epd_disp_bitmap("RAIN.BMP",360, 200);
  epd_udpate();

  delay(5000);
}

void get_Data(){//получаем данные с датчиков
  Temp = bme.readTemperature();
  Pressure = (bme.readPressure()/ 100.0F);
  Humidity = bme.readHumidity();
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
  TimeAndWeather();
  Time = digitalClock();
  Weather = WeatherDisplay();
  draw_text(String(Temp), String(Humidity), String(CO2), String(Pressure), String(Time), String(Weather));
}

void send_Data(){//отправляем на mqtt (по запросу)
  mqtt_client.publish(top1,String(Temp));
  mqtt_client.publish(top2,String(Pressure));
  mqtt_client.publish(top3,String(Humidity));
  mqtt_client.publish(top4,String(CO2));
  mqtt_client.publish(top5,String(Time));
  mqtt_client.publish(top6,String(Weather));
}
