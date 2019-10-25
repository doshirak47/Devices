

#define PIN D5

void call(String& top, String& val){//if(top == device_name + "/"){};
  if(top == device_name + "/light"){
      Serial.println(val.toInt());
      analogWrite(PIN, val.toInt());
    };
}

void setup_topic(){//mqtt_client.subscribe(String(id)+"/");
  mqtt_client.subscribe(String(id)+"/light");
}

void setup_external(){//pinMode();
  pinMode(PIN,OUTPUT);
}

int main(){//mqtt_client.publish(device_name + "/", "");
}
