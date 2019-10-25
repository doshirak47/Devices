#define PIN D5
#define PWM_R 1023
void call(String& top, String& val){//if(top == device_name + "/"){};
  if(top == device_name + "/light"){
    v = val.toInt();
    if((v >= 0)&&(v<=PWM_R){
      analogWriteRange(PWM_R);
      analogWrite(PIN, val.toInt());
    }
  }
}

void setup_topic(){//mqtt_client.subscribe(String(id)+"/");
  mqtt_client.subscribe(String(id)+"/light");
}

void setup_external(){//pinMode();
  pinMode(PIN,OUTPUT);
  
}
