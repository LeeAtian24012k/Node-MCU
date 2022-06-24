#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h" //https://github.com/tzapu/WiFiManager
#include <Wire.h> //Thư viện I2C
#include <WebSocketsClient.h> //https://github.com/Links2004/arduinoWebSockets
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Time.h>

WiFiUDP u;
NTPClient n(u, "pool.ntp.org",7*3600);
WebSocketsClient webSocket;

const String ip_host = "ahkiot.herokuapp.com";
const uint16_t port = 80;
int reqServer = 0;
int temp = 0;
int tempErr = 0;
String Verify;
String resServer = "";
int t = 11*3600 + 47*60;
String DateLine = "";
String TimeClose = "";
String TimeOpen = "";
String FaceRecognition = "";
String RealTimeClock;
//String DateLine = "2022-06-09";
//String TimeClose = "21:60";
//String TimeOpen = "21:55";
//String FaceRecognition = "FACE_RECOGNITION_ON";

void configModeCallback (WiFiManager *myWiFiManager)
{
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:

    Serial.printf("[WSc] Disconnected!\n");

    break;

  case WStype_CONNECTED:

    Serial.printf("[WSc] Connected to url: %s\n", payload);

    break;

  case WStype_BIN:

    webSocket.sendTXT("NodeMCU ESP8266 connected!");

    Serial.println((char *)payload);

    String str = (char*)payload;
    
    String p;

    Serial.println(str.length());
    DateLine = "";
    TimeClose = "";
    TimeOpen = "";
    FaceRecognition = "";
    for(int i=0; i<str.length(); i++){
      if(str.length()==42){
        if(i<10){
          DateLine.concat(str[i]);
//          Serial.println(DateLine);
        }
        else if(10<i and i<16){
          TimeOpen.concat(str[i]); 
//          Serial.println(TimeOpen);
        }
        else if(16<i and i<22){
          TimeClose.concat(str[i]); 
//          Serial.println(TimeClose);
        }
        else if(22<i and i<42){
          FaceRecognition.concat(str[i]); 
//          Serial.println(FaceRecognition);
        }
      }
      else if(10<str.length() && str.length()<42){
        if(i<24){
          p.concat(str[i]);
//          Serial.println(p.substring(0,24));
        }
      }
    }

    Serial.println(FaceRecognition.substring(0,19));
      
    if (strcmp((char *)payload, "MANUAL_ON") == 0)
    {
      reqServer = 1;
    }
    else if (strcmp((char *)payload, "MANUAL_OFF")==0)
    {
      reqServer = -1;
    }
    else if (strcmp((char *)payload, "AUTO_ON") == 0)
    {
      reqServer = 2;
    }
    else if (strcmp((char *)payload, "AUTO_OFF") == 0)
    {
      reqServer = -2;
    }
    else if ((p.substring(0,24) == "FACE_RECOGNITION_CONFIRM") && (Verify == "confirm"))
    {
      reqServer = 4;
    }
    else if (FaceRecognition.substring(0,19) == "FACE_RECOGNITION_ON")
    {
      reqServer = 3;
    }
    else if (strcmp((char *)payload, "FACE_RECOGNITION_OFF") == 0)
    {
      reqServer = -3;
      DateLine = "";
      TimeClose = "";
      TimeOpen = "";
      FaceRecognition = "";
    }
    if (strcmp((char *)payload, "FACE_RECOGNITION_CONFIRM-VuTuanHung") == 0)
    {
      reqServer = 4;
    }
    break;
  }
}

void setup()
{
  Serial.begin(9600);
  Wire.begin(D1, D2);
  Serial.println("ESP8266-WebSocketClient");
  //Khai bao wifiManager
  WiFiManager wifiManager;
  //Setup callback de khoi dong AP voi SSID "ESP+chipID"
  wifiManager.setAPCallback(configModeCallback);
  if (!wifiManager.autoConnect())
  {
    Serial.println("failed to connect and hit timeout");
    //Neu ket noi that bai thi reset
    ESP.reset();
    delay(1000);
  }
  // Thanh cong thi bao ra man hinh
  Serial.println("connected...");

  webSocket.begin(ip_host, port);
  webSocket.onEvent(webSocketEvent);
}
void loop()
{
  webSocket.loop();
  Wire.beginTransmission(8);
  Wire.write(reqServer);
  Wire.endTransmission();
  Wire.requestFrom(8, 13);
  if(reqServer==4){
     reqServer = 0;
  }
  n.begin();
  n.update();
  int a = n.getHours();
  int b = n.getMinutes();
  String h = String(a);
  String m = String(b);

  if (b<10){
    RealTimeClock = String(h+":"+"0"+m);
  }
  else{
    RealTimeClock = String(h+":"+m);
  }
//  Serial.println(RealTimeClock);
  time_t epochTime = n.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  String CurrentMonth;
  String MonthDay;
//  if (monthDay < 10)
//  {
//    MonthDay = String("0"+String(monthDay));
//  }
//  else{
//    MonthDay = monthDay;
//  }
  if (currentMonth < 10)
  {
    CurrentMonth = String("0"+String(currentMonth));
  }
  else{
    CurrentMonth = currentMonth;
  }
  int currentYear = ptm->tm_year+1900;
  String currentDate = String(currentYear) + "-" + CurrentMonth + "-" + String(monthDay);
//  Serial.println(DateLine.substring(0,10));
//  Serial.println(TimeOpen.substring(0,5));
//  Serial.println(TimeClose.substring(0,5));
//  Serial.println(RealTimeClock);
//  Serial.println(currentDate);
  
  if (currentDate == DateLine.substring(0,10)){
//    Serial.println(DateLine.substring(0,10));
//    Serial.println(currentDate);
      if (RealTimeClock == TimeOpen.substring(0,5)){
//        Serial.println(TimeOpen.substring(0,5));
//        Serial.println(RealTimeClock);
        Verify = "confirm";
        Serial.println(Verify);
      }
      else if (RealTimeClock == TimeClose.substring(0,5)){
//        Serial.println(TimeClose.substring(0,5));
        Verify = "";
        Serial.println(Verify);
      }
  }


  if (Wire.available())
  {
    int reqArduino = Wire.read();
    if (reqArduino == 1 && temp == 0)
    {
      resServer = "open successfully";
      Serial.println(reqArduino);
      temp = 1;
      tempErr = 1;
    }
    else if (reqArduino == 2 && temp == 1)
    {
      resServer = "close successfully";
      Serial.println(reqArduino);
      temp = 0;
      tempErr = 0;
    }
    else if (reqArduino == 255 && tempErr == 0)
    {
      resServer = "close error";
      Serial.println(reqArduino);
      temp = 1;
      tempErr = 1;
    }
    else if (reqArduino == 254 && tempErr == 1)
    {
      resServer = "open error";
      Serial.println(reqArduino);
      temp = 0;
      tempErr = 0;
    }
  }
  if (resServer != "")
  {

    webSocket.sendTXT(resServer);

    resServer = "";
  }
}
