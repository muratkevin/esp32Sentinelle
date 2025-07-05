#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include "DHT.h"
#include <ESP32Servo.h>
#include <SoftwareSerial.h>
#include <SparkFun_GridEYE_Arduino_Library.h>
#include <DFRobotDFPlayerMini.h> 
#define PIN_SERVO 32 
#define DHTPIN 12
#define DHTTYPE DHT11
#define sensorPin 36

Servo MonServo;
GridEYE grideye;
DFRobotDFPlayerMini myDFPlayer ; 
DHT dht(DHTPIN, DHTTYPE);

SoftwareSerial SoftSerial(18, 19);
SoftwareSerial mySoftwareSerial(23, 5);

const char* ssid     = "Sentinelle";
const char* password = "azertyui";

int ID = 1;
float temp = 0;
String fumee = "Fumee";
bool Obstacle = false;
String emplacement = "Point d'arrêt 0 ";
String direction = "Nord";
float hotPixelValue = 0;
int hotPixelIndex = 0;
float testPixelValue = 0;
String    bufferString="";
uint8_t   count=0;
int boucle = 1;
short data = 120;
int tour = 0;
int etat = digitalRead(26);
bool a = false;
bool b = false;
bool c = false;
bool d = false;




const char* serverName = "http://192.168.0.102/sentinelle_post-data.php";

String apiKeyValue = "tPmAT5Ab3j7F9";


void setup() {
  SoftSerial.begin(9600);
  mySoftwareSerial.begin(9600) ;
  Serial.begin(9600);
  dht.begin();
  Wire.begin();
  grideye.begin();
  myDFPlayer.begin(mySoftwareSerial);
  MonServo.setPeriodHertz(50); 
  MonServo.attach(PIN_SERVO, 500, 2400);
  pinMode(26, INPUT);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
 
RFID();

if (tour == 0 && boucle == 0){
tour = 1;
for (int pos = 180; pos >= 0; pos--) {
    MonServo.write(pos);
    delay(50);
  }
  delay(200);
for (int pos = 0; pos <= 60; pos += 1) {
    MonServo.write(pos);
    delay(80);
  }

  CAM_IR();
  readTEMP();
  readFumee();
  Envoi_BDD();
  Alerte();

  delay(2000);
}

if (tour == 1 && boucle == 0){
tour = 2;

for (int pos = 60; pos <= 120; pos += 1) {
    MonServo.write(pos);
    delay(80);
  }

  CAM_IR();
  readTEMP();
  readFumee();
  Envoi_BDD();
  Alerte();

  delay(2000);
}

if (tour == 2 && boucle == 0){
tour = 0;
boucle = 1;
bufferString="";

for (int pos = 120; pos <= 180; pos += 1) {
    MonServo.write(pos);
    delay(80);
  }

  CAM_IR();
  readFumee();
  readTEMP();
  Envoi_BDD();
  Alerte();

  delay(2000);
}

}

void readTEMP() {
temp = dht.readTemperature();
}

void readFumee() {

int outputValue = analogRead(sensorPin);

if (outputValue > 65){
  fumee = "Fumée";
  }
else {
  fumee = "Pas de fumée";
}
}

void Envoi_BDD() {

  if(WiFi.status()== WL_CONNECTED){
    WiFiClient client;
    HTTPClient https;
    
    https.begin(client, serverName);
    

    https.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    String httpRequestData = "api_key=" + apiKeyValue + "&Point_chaud=" + hotPixelValue
                          + "&Fumee=" + fumee + "&Temperature=" + temp + "&Direction=" + direction + "&Emplacement=" + emplacement + "&IDrobot=" + ID + "&Point_chaudd=" + hotPixelValue + "&Emplacementt=" + emplacement + "&Obstacle=" + Obstacle + "" ;
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);
    
    int httpResponseCode = https.POST(httpRequestData);
    
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    https.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }

  delay(3000); 

}

void RFID(){
if( SoftSerial.available() ){
        bufferString="";
        count=0;
      
      while(SoftSerial.available()){ 
        delay(5);
        char newChar = SoftSerial.read();
        if(count>2 && count<11){
          bufferString += newChar;
        }     
        count++;
        if(bufferString.length()==14){ break; }
      }

    }   

    if(bufferString=="0029A8DC" && a == false){
      boucle = 0;
      a = true;
      data = 122;
      d = false;
      emplacement = "Point d'arrêt 1";} 
      else {data = 120;}
      Serial.write(data);

    if(bufferString=="007A35E9" && b == false){
      boucle = 0;
      data = 123; 
      b = true;
      a = false;
      emplacement = "Point d'arrêt 2";}
      else {data = 120;}
      Serial.write(data);
  
    if(bufferString=="008B0BF0" && c == false){
      boucle = 0;
      data = 124;
      c = true;
      b = false;
      emplacement = "Point d'arrêt 3";}
      else {data = 120;}
      Serial.write(data);
    
    if(bufferString=="00C73C86" && d == false){
      boucle = 0;
      data = 125;
      d = true;
      c = false; 
      emplacement = "Point d'arrêt 4";}else {data = 120;} 
      Serial.write(data);    
   
}

void CAM_IR() {

testPixelValue = 0;
hotPixelValue = 0;
hotPixelIndex = 0;

 for(unsigned char i = 0; i < 64; i++){
    testPixelValue = grideye.getPixelTemperature(i);
      if(testPixelValue > hotPixelValue){
        hotPixelValue = testPixelValue;
        hotPixelIndex = i;
      }
  }

}


void Alerte() {

while(hotPixelValue >= 60){
  int etat = digitalRead(26);
  myDFPlayer.setTimeOut(500) ;
  if (etat == HIGH) {
  myDFPlayer.stop();
  break;} else{myDFPlayer.play(1);}
}

}





