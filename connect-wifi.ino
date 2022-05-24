#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h" //https://github.com/tzapu/WiFiManager
#include <Wire.h> //Thư viện I2C
#include <WebSocketsClient.h> //https://github.com/Links2004/arduinoWebSockets
#include <WiFiUdp.h>
#include <NTPClient.h>

WiFiUDP u;
NTPClient n(u, "1.asia.pool.ntp.org", 7*3600);
WebSocketsClient webSocket;

const String ip_host = "ahkiot.herokuapp.com";
const uint16_t port = 80;
int reqServer = 0;
int temp = 0;
int tempErr = 0;
String resServer = "";
int t = 11*3600 + 47*60;
int a, b;

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
    std::string str(payload);
    for (int i=0;i<=10;i++)
    {
     Serial.println(str); 
    }
//    Serial.println((char *)payload[0]);
//    Serial.println((char *)payload[1]);
//    Serial.println((char *)payload[2]);
    n.update();
    a = n.getHours();
    b = n.getMinutes();
    String h = String(a);
    String m = String(b);
    String myString = String(h+":"+m);
    if (myTime == myString)
    {
      reqServer = 1;
    }
    else if (strcmp((char *)payload, "MANUAL_ON")==0)
    {
      reqServer = 1;
    }
    else if (strcmp((char *)payload, "MANUAL_OFF") == 0)
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
    else if (strcmp((char *)payload, "FACE_RECOGNITION_ON") == 0)
    {
      reqServer = 3;
    }
    else if (strcmp((char *)payload, "FACE_RECOGNITION_OFF") == 0)
    {
      reqServer = -3;
    }
    else if (strcmp((char *)payload, "FACE_RECOGNITION_CONFIRM") == 0)
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
  n.begin();
}
void loop()
{
  webSocket.loop();
  Wire.beginTransmission(8);
  Wire.write(reqServer);
  Wire.endTransmission();
  Wire.requestFrom(8, 13);
  reqServer = 0;
  
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
      resServer = "open error";
      Serial.println(reqArduino);
      temp = 1;
      tempErr = 1;
    }
    else if (reqArduino == 254 && tempErr == 1)
    {
      resServer = "close error";
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
//  n.update();
//  a = n.getHours();
//  b = n.getMinutes();
//  String h = String(a);
//  String m = String(b);
//  String myString = String(h+":"+m);
//  if (myTime == myString)
//  {
//    Serial.println("success");
//  }
//  Serial.println(n.getFormattedTime());
//  delay(1000);
}
