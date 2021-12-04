#include <EEPROM.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

//++++++++++НАСТРОЙКИ++++++++++//
#define MASTER_WRITE false
#define MASTER_HASH "1d6e126e3707e5ed51c5eea083e82bf4"

#define YOUR_SSID "F"                                       //Логин сети F                   Keenetic-4642
#define YOUR_PASS "rhben_rhben+"                            //Пароль сети rhben_*****+       BhBJA5vi
#define SERVER_LOCATION "http://192.168.1.50:25565/"        //Куда посылать запрос http://192.168.1.50:25565/   http://192.168.1.35:8080/

#define CONTENT_TYPE "application/json"           //MEMI-тип запроса text/plain
#define BLOCK1 60                                 //Диапозон блоков с которыми работаем, ниж. граница
#define BLOCK2 61                                 //Диапозон блоков с которыми работаем, верх. граница
#define SECRET "777888999"                        //Пароль доступа куда шлём

#define RST_PIN 5      //Пин куда подключаем RST
#define SS_PIN 15      //Пин куда подключаем SDA
#define E_ADDRESS 32   //Адрес ячейки в EPROOM, где записан ID esp
//++++++++++НАСТРОЙКИ++++++++++//

boolean Master = false;
String MasterHash = "nill";
String new_hash;
String IDesp;

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

StaticJsonDocument<200> Post_Json(String Info, String card){
  StaticJsonDocument<200> SendJ;
  SendJ["Card"] = card;
  SendJ["IDesp"] = IDesp;
  SendJ["Info"] = Info;
  SendJ["Pass"] = SECRET;
  Serial.print("Finished json line: "); serializeJson(SendJ, Serial); Serial.println();
  String StrSendJ;
  serializeJson(SendJ, StrSendJ);

  WiFiClient client;
  HTTPClient http;
  http.begin(client, SERVER_LOCATION);
  http.addHeader("Content-Type", CONTENT_TYPE);
  int httpCode = http.POST(StrSendJ);
  String StrAcceptJ = http.getString();
  Serial.print("Accepted json line: "); Serial.println(StrAcceptJ);
  if (httpCode != 200) {Serial.print("Http code: "); Serial.println(httpCode);}
  http.end();
    
  StaticJsonDocument<200> AcceptJ;
  DeserializationError error = deserializeJson(AcceptJ, StrAcceptJ);
  if (error) {Serial.print("DeserializeJson failed: "); Serial.println(error.c_str());}
  return AcceptJ;
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  EEPROM.begin(512);
  mfrc522.PCD_Init();
  WiFi.begin(YOUR_SSID, YOUR_PASS);
  
  //++++++++++ПОДКЛЮЧЕНИЕ К WiFi++++++++++//
  Serial.println(); Serial.print("Waiting for connection");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(50);
  }
  Serial.println(); Serial.println("Wifi is connect!");
  //++++++++++ПОДКЛЮЧЕНИЕ К WiFi++++++++++//
  
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF; //Генерация ключа для mfrc522
  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); Serial.println(); //Вывод инфы о ридере
  
  //++++++++++НАСТРОЙКА IDesp++++++++++//
  if (WiFi.status() == WL_CONNECTED) {
    IDesp = EEPROM.read(E_ADDRESS);
    StaticJsonDocument<200> doc = Post_Json("GiveIDesp", "nill");
    EEPROM.write(E_ADDRESS, doc["~IDesp"].as<byte>());
    EEPROM.commit(); 
    IDesp = EEPROM.read(E_ADDRESS);
    Serial.print("Success Esp ID: "); Serial.println(IDesp); Serial.println();
  } else Serial.println("Error in WiFi connection! Can't setup IDesp!");
  //++++++++++НАСТРОЙКА IDesp++++++++++//

  //++++++++++ПОЛУЧЕНИЕ MasterHash++++++++++//
  if (WiFi.status() == WL_CONNECTED) {
    if (MASTER_WRITE) {
      StaticJsonDocument<200> doc = Post_Json("GiveMasterHash", "nill");
      MasterHash = doc["~Card"].as<String>();
      Serial.print("Success Master Hash: "); Serial.println(MasterHash); Serial.println();
    }
  } else Serial.println("Error in WiFi connection! Can't setup MASTER_HASH!");
  //++++++++++ПОЛУЧЕНИЕ MasterHash++++++++++//
}


void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      if (MASTER_WRITE) Write_hash(MasterHash);
      else if (Master) {Write_hash(new_hash); Master = false;}
      else{
        String data = Read_hash();
        StaticJsonDocument<200> doc = Post_Json("SwipeCard", data);
        Master = doc["~Info"].as<String>() == "Master";
        new_hash = doc["~Card"].as<String>();
        Serial.println();
      }
    }
  } else {
    Serial.println("Error in WiFi connection!");
    delay(500);
  }
}
