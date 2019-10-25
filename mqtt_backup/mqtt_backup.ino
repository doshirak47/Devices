#include <ESP8266WiFi.h>            //https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi
#include <WiFiClient.h>
#include <PubSubClient.h>           //https://github.com/Imroy/pubsubclient

const char *id = "1";

const char *ssid = "MikroTik-7931DC"; // Имя роутера
const char *password = ""; // Пароль роутера

String mqtt_server = "m11.cloudmqtt.com"; // Имя сервера MQTT
int mqtt_port = 11708; // Порт для подключения к серверу MQTT
String mqtt_user = "ddzdsmmr"; // Логин для подключения к серверу MQTT
String mqtt_pass = "9J1XyqzqSLc_"; // Пароль для подключения к серверу MQTT
String device_name = id;//имя  device_name

WiFiClient WiFi_client; 
PubSubClient mqtt_client(WiFi_client, mqtt_server, mqtt_port);
 

void callback(const MQTT::Publish& pub){
     String payload = pub.payload_string();
     String topic = pub.topic(); 
     Serial.print("<\nnew message: \ntopic: \"");
     Serial.print(topic);
     Serial.print("\"\npayload :\n");
     Serial.print(payload);
     Serial.print("\n>\n");
     
}

void setup_wifi(){
     Serial.print("\nConnecting to ");
     Serial.println(ssid);
     WiFi.begin(ssid, password);
     while (WiFi.status() != WL_CONNECTED) {
            delay(300);
            Serial.print(".");
           }
     Serial.println("\nWiFi connected");
     Serial.println("IP address: ");
     Serial.println(WiFi.localIP());
}

void setup_mqtt(){
    if (!mqtt_client.connected()) {
      Serial.print("Connecting to MQTT server ");
      Serial.print(mqtt_server);
      Serial.println("...");
      if (mqtt_client.connect(MQTT::Connect("ard").set_auth(mqtt_user, mqtt_pass))) {
        Serial.println("Connected to MQTT server \n");
        mqtt_client.set_callback(callback);
        // подписываемся под топики
        mqtt_client.subscribe(String(id)+"/#");
      } else {
        Serial.println("Could not connect to MQTT server"); 
      }
    }
}


void setup(void) {
     Serial.begin(115200);
     Serial.print("\n<=========================>\nStart device, id: ");
     Serial.println(id);
     setup_wifi();
     if (WiFi.status() == WL_CONNECTED) setup_mqtt();
}

void loop(void){
  if (WiFi.status() == WL_CONNECTED) {
     if (mqtt_client.connected()){
         mqtt_client.loop();
      }else{
        Serial.println("MQTT Error, try reconect:"); 
        setup_mqtt();
      }
  }else{
    Serial.println("WiFi Error, try reconect:"); 
    setup_wifi();
    setup_mqtt();
  }
}
