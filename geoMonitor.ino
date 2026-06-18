#include <LiquidCrystal_I2C.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

LiquidCrystal_I2C lcd (0x27, 16, 2);

#define pinEcho 19
#define pinTrig 18

#define pinPiezoVer 33
#define pinPiezoHor 32
#define pinLembab 35
#define pinSuhu 13

OneWire oneWire(pinSuhu);
DallasTemperature sensors(&oneWire);

int interval = 30000;
long millisCurr;
long millisPrev;

int getarCurr, getarPrev;
int sel = 150;

float suhu, SR, kelembaban, dis;
int anGetar, anLembab, anNoiseVer, anNoiseHor;
long duration;

void setup() {
  Serial.begin(115200);
  
  lcd.init();
  lcd.backlight();

  pinMode(pinEcho,INPUT);
  pinMode(pinTrig,OUTPUT);

  pinMode(pinLembab,INPUT);
  pinMode(pinPiezoVer,INPUT);
  pinMode(pinPiezoHor,INPUT);
  
  Serial.println("MONITORING TANAH");
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("MONITORING TANAH");
  lcd.setCursor(2,1);
  lcd.print("GET  STARTED");
  delay(1000);
  
  konekWifi();
  
  lcd.clear();

  sensors.begin();
  millisCurr = millis();
  getarCurr = anGetar;
}

void konekWifi(){
  WiFi.mode(WIFI_STA);
  WiFiManager wm;
//  wm.resetSettings();

  bool res;

  res = wm.autoConnect("MONAH","monah123");
  
  Serial.print("RES: ");Serial.println(res);
  if(!res) {
    Serial.println("GAGAL CONNECT");
    
    lcd.setCursor(0,0);
    lcd.print("WIFI BELUM ADA ");
  } 
  else { 
    Serial.println("WIFI TERKONEKSI");
    
    lcd.setCursor(0,0);
    lcd.print("WIFI TERKONEKSI");
  } 
}

void loop() {
  getNoise();
  Serial.print("Noise Ver   : "); Serial.println(anNoiseVer);
  Serial.print("Noise Hor   : "); Serial.println(anNoiseHor);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Ver : "); lcd.print(anNoiseVer);
  lcd.setCursor(0,1);
  lcd.print("Hor : "); lcd.print(anNoiseHor);
  delay(500);
  
  if(anNoiseVer > 0 || anNoiseHor > 0){
    kirimData();
  }
  
  getDis();
  Serial.print("Jarak       : "); Serial.println(dis);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print(dis);lcd.print("cm ");
  
  getTemper();
  Serial.print("suhu        : "); Serial.println(suhu);
  lcd.print(suhu);lcd.print((char)223);lcd.print("C"); 
  
  getSoil();
  Serial.print("Lembab      : "); Serial.println(anLembab);
  Serial.print("Kelembaban  : "); Serial.println(kelembaban);
  lcd.setCursor(0,0);
  lcd.print(kelembaban);lcd.print("%");  
  Serial.println("========================");
  delay(500);
  
  millisCurr = millis();
  if(millisCurr - millisPrev > interval){
    kirimData();
    millisPrev = millisCurr;
  }
}

void getNoise(){
  anNoiseVer = analogRead(pinPiezoVer);
  anNoiseHor = analogRead(pinPiezoHor);
}

void getSoil(){
  //Baca sensor Kelembaban
  anLembab = analogRead(pinLembab);
  kelembaban = map(anLembab, 1500, 4095, 100, 0);
//  lcd.setCursor(0,0);
//  lcd.print("KELEM: "); 
//  lcd.print(" ");lcd.print(kelembaban);lcd.print("%"); 
}

void getTemper(){
  //Baca sensor Suhu
  sensors.requestTemperatures();
  suhu = sensors.getTempCByIndex(0);
  
  lcd.setCursor(0,1);
//  lcd.print("SUHU : "); 
//  lcd.print(suhu);lcd.print((char)223);lcd.print("C"); 
}

void getDis(){
  //Baca Jarak
  digitalWrite(pinTrig,LOW);
  delayMicroseconds(2);
  digitalWrite(pinTrig,HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig,LOW);

  duration = pulseIn(pinEcho,HIGH);
  dis = duration * 0.0344 / 2;

//  lcd.print(" "); lcd.print(dis);lcd.print("cm");
}

String serverMonah = "https://monah.proaction-palu.com/InsertDB.php";

void kirimData(){
  if(WiFi.status() == WL_CONNECTED){
      lcd.clear();
      WiFiClient client;
      HTTPClient http;

      String httpReqData = serverMonah + "?noiseH=" + String(anNoiseHor) +"&noiseV=" + String(anNoiseVer) +
                          "&kelembaban=" + String(kelembaban) + "&suhu=" + String(suhu) + "&jarak=" + String(dis);

      http.begin(httpReqData.c_str());

//      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      Serial.print("Req Data: "); Serial.println(httpReqData);

      int httpResponseCode = http.GET();
      String payload = http.getString();
      
      lcd.setCursor(0,0);
      lcd.print(httpResponseCode);
      
      if(httpResponseCode > 0){
        Serial.print("HTTP RESPONSE CODE: "); Serial.println(httpResponseCode);
        Serial.println(payload);
        Serial.println("DATA TERKIRIM");

        lcd.setCursor(0,1);
        lcd.print("DATA TERKIRIM");
      }
      else{
        Serial.print("ERROR CODE: "); Serial.println(httpResponseCode);
        Serial.println(payload);
        Serial.println("GAGAL TERKIRIM");
        
        lcd.setCursor(0,1);
        lcd.print("GAGAL TERKIRIM");
      }
      http.end();
  }
  delay(1000);
}
