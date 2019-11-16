#include <SPI.h>
#include <LoRa.h>
#include <TroykaMQ.h>

const int analogSignal = A0; //подключение аналогового сигналоьного пина
const int digitalSignal = 8; //подключение цифрового сигнального пина
boolean noGas; //переменная для хранения значения о присутствии газа
int gasValue = 0; //переменная для хранения количества газаconst int analogSignal = A0; //подключение аналогового сигналоьного пина
const int digitalSignal = 8; //подключение цифрового сигнального пина
boolean noGas; //переменная для хранения значения о присутствии газа
int gasValue = 0; //переменная для хранения количества газа

int counter = 0;

void setup() {
  
  pinMode(digitalSignal, INPUT); //установка режима пина
  Serial.begin(9600); //инициализация Serial порта
  
  while (!Serial);

  Serial.println("LoRa Sender");

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  LoRa.setTxPower(10);
  
}

void loop() {

  noGas = digitalRead(digitalSignal); //считываем значение о присутствии газа
  gasValue = analogRead(analogSignal); // и о его количестве
  
  Serial.print("Sending packet: ");
  Serial.println(counter);

  //отправка пакета
  LoRa.beginPacket();
  LoRa.print("There is ");
  if (noGas) {
    LoRa.print("no gas");
  }
  else {
    LoRa.print("gas");
  }
  LoRa.print(", the gas value is ");
  LoRa.println(gasValue);//вывод сообщения
  LoRa.endPacket();
  
 

  delay(5000);
}
