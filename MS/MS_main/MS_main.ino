#define MQTT_CLIENT_TEST 1
#include <ESP8266WiFi.h>            //https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi
#include <WiFiClient.h>
#include <PubSubClient.h>           //https://github.com/Imroy/pubsubclient

const char *id = "M01";

const char *ssid = "MikroTik-7931DC"; // Имя роутера
const char *password = ""; // Пароль роутера

#if MQTT_CLIENT_TEST == 1
String mqtt_server = "farmer.cloudmqtt.com"; // Имя сервера MQTT
int mqtt_port = 14626; // Порт для подключения к серверу MQTT
String mqtt_user = "fjllmrfi"; // Логин для подключения к серверу MQTT
String mqtt_pass = "EFRUJQ1S0Dmt"; // Пароль для подключения к серверу MQTT 
#else
IPAddress mqtt_server(192, 168, 88, 98);
int mqtt_port = 1024; // Порт для подключения к серверу MQTT
String mqtt_user = "ddzdsmmr"; // Логин для подключения к серверу MQTT
String mqtt_pass = "9J1XyqzqSLc_"; // Пароль для подключения к серверу MQTT
#endif

String device_name = id;//имя  device_name
String device_type = "no_type";

WiFiClient WiFi_client;
WiFiClient WiFi_client2;  
PubSubClient mqtt_client(WiFi_client, mqtt_server, mqtt_port);
 

void callback(const MQTT::Publish& pub){
     String payload = pub.payload_string();
     String topic = pub.topic(); 
     Serial.print("<\nnew message: \ntopic: \"");
     Serial.print(topic);
     Serial.print("\"\npayload :\n");
     Serial.print(payload);
     Serial.print("\n>\n");
     call(topic, payload);
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
      #if MQTT_CLIENT_TEST == 1
       if (mqtt_client.connect(MQTT::Connect(device_name).set_auth(mqtt_user, mqtt_pass))) 
      #else
       if (mqtt_client.connect(device_name))
      #endif
      {
        Serial.println("Connected to MQTT server \n");
        mqtt_client.set_callback(callback);
        // подписываемся под топики
        setup_topic();
      } else {
        Serial.println("Could not connect to MQTT server"); 
      }
    }
}


void setup(void) {
     Serial.begin(115200);
     Serial.print("\n<=========================>\nStart device, id: ");
     Serial.println(id);
     setup_external();
     setup_wifi();
     if (WiFi.status() == WL_CONNECTED) setup_mqtt();
}

void loop(void){
  if (WiFi.status() == WL_CONNECTED) {
     if (mqtt_client.connected()){
         mqtt_client.loop();
         get_Data();
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
