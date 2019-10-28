#define PIN1 D2
#define PIN2 D6
#define PWM_R 100

String L1 = "/light1";
String L2 = "/light2";

int curValue1 = 0;
int curValue2 = 0;

void call(String& top, String& val){//if(top == device_name + "/"){};
  if(top == device_name + L1){
    set_brightness(val,0);
    mqtt_client.publish(top + "/response",val);
  }
  if(top == device_name + L2)
    set_brightness(val,1);
}
void set_brightness(String& val, int pin){
  int v = val.toInt();
  int cur = pin?curValue2:curValue1;
  int step = (v-cur>0)?1:-1;
  if((v >= 0)&&(v<=PWM_R)){
    analogWriteRange(PWM_R);
    while(cur!=v){
      cur+=step;
      analogWrite(pin?PIN2:PIN1, cur);
      delay(5);
    }
    pin?curValue2:curValue1 = cur;
  }
}

void setup_topic(){//mqtt_client.subscribe(String(id)+"/");
  mqtt_client.subscribe(device_name+L1);
  mqtt_client.subscribe(device_name+L2);
}

void setup_external(){//pinMode();
  pinMode(PIN1,OUTPUT);
  pinMode(PIN2,OUTPUT);
}
