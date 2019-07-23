/****************************************************************
'*  Name    : ESP8266-Wifi_Setup_Via_SoftAP.INO                 *
'*  Author  : Joseph Cottingham and Ethan Reiland               *
'*  Notice  : Copyright (c) 2019                                *
'*          : All Rights Reserved                               *
'*  Date    : 6/27/2019                                         *
'*  Version : 1.0                                               *
'*  Notes   :                                                   *
'*          :                                                   *
'****************PROGRAM DESCRIPTION*********************************************************  
'*          : This firmware allows the ESP8266 to take in network configurations and setup a*
'*          : IOT heartbeat return system for environment data factors such as water level  *
'****************CHANGES FROM PREVIOUS REVISION**********************************************
'*          :                                                                               *
'* **************IMPORTANT CONSIDERATIONS FOR THE ESP8266************************************
'*          :                                                                               *
'*********************************************************************************************************************************** 
'*          :  Arduino Programmer Settings for ESP8266 Plugin                                                                      *
'*          :  Oscillator Frequency: 26 MHz                                                                                        *
'*          :  CPU Frequency: 80 MHz                                                                                               *
'*          :  Flash Size: 2M (128K SPIFFS)                                                                                        *
'*          :  Upload Baud: 115200                                                                                                 *
'*          :  Restart Meathod: ck                                                                                                 *
'*          :  Flash Mode: DOUT                                                                                                    *
'*          :  Flash Frequency: 40MHz                                                                                              *
'*          :  VTables: Flash                                                                                                      *
'*          :  Board: Generic ESP8266  Module                                                                                      *
'***********************************************************************************************************************************                                                                                                    
'*                                          *     
'******************************************************************************************************************
'*          :PROGRAMMING HARDWARE SETUP (ESP8266 must be powered by Adapter or Battery Power)                     *
'* GPIO16   :Pin 0: N/A                                                                                           *
'* GPIO14   :Pin 1: N/A                                                                                           *
'* GPIO12   :Pin 2: N/A                                                                                           *
'* GPIO13   :Pin 3: Mem Rest Momentary Switch                                                                     *
'* GPIO15   :Pin 4: Pull Down For Programing and normal operation                                                 *
'* GPIO2    :Pin 5: N/A                                                                                           *
'* GPIO0    :Pin 6: Pull Down for normal operation and high for programing                                        *
'* GPIO4    :Pin 7: NS7000 TX                                                                                     *
'* GPIO5    :Pin 8: NS7000 RX                                                                                     *
'* RXD      :Pin 9:  Device RX Programing                                                                         *
'* TXD      :Pin 10: Device TX Programing                                                                         *
'*          :Rest  : Momentary Switch that brings it low                                                          *
'*          :ADC   : N/A                                                                                          *
'*          :CH_PD : Pulled High for programing and operation                                                     *
'*          :GND   : Pull Down to Negative                                                                        *
'*          :VCC   : Pull Up to Postive                                                                           *
******************************************************************************************************************/

#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <ESPSoftwareSerial.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#ifndef APSSID
#define APSSID "H2O-WiFi-Unit"
#define APPSK  ""
#endif

WebServer server(80);
WiFiClient tcpClient;
SoftwareSerial ESPserial(17, 16);
WiFiClientSecure httpsClient;
PubSubClient mqttClient(httpsClient);

void connectAWSIoT();
void mqttCallback (char* topic, byte* payload, unsigned int length);

const int port = 8883;
const char *endpoint = "a3jmxo5vrdefm0-ats.iot.us-east-2.amazonaws.com";
char *pubTopic = "Heartbeat";

const char* rootCA = "-----BEGIN CERTIFICATE-----\n" \
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
"rqXRfboQnoZsG4q5WTP468SQvvG5\n"
"-----END CERTIFICATE-----\n";

const char* certificate = "-----BEGIN CERTIFICATE-----\n" \
"MIIDWjCCAkKgAwIBAgIVAOSBIDNhxZ/c6kQyJn/yjm/2txy7MA0GCSqGSIb3DQEB\n"
"CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\n"
"IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0xOTA2MjYxODMw\n"
"MTZaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh\n"
"dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCmM5B/oLbYyf7zZQOf\n"
"M8Nk3nEDUCf041LJfwlo1AaILctHSaKiILibnJDv9t9bQHqBFIAv7CKV2MFhp9FH\n"
"HQYVNTIR/MDStqwL0qFdb6qM7Px6tsYz8bknBeBdn93cTyI4kaa+cUp3GfYtYxtH\n"
"ybTTZtMVgxZOiDgPe5pR8TSr4NAD3zu6IPb3RPaH6R9yWIgGuJpOKFEmnYnZXBeO\n"
"Z8GfJYOuwuUCel7I5r0vNajG4e8CsqS/LjnQYBxuNYjZ+IRbqcMd7VXGAsyzc/i6\n"
"t0mAK5QW0bW+bI/8rCSZ7QJzCPGrL5JCWpc0JfaeWrbD4oqTEylWWvW2IiSD8kea\n"
"0QBtAgMBAAGjYDBeMB8GA1UdIwQYMBaAFCu1E7oWbF9lqVrjhtrPuUqICjk5MB0G\n"
"A1UdDgQWBBQWrsJgdTba/gsp7Aw9+I7EOVXn1TAMBgNVHRMBAf8EAjAAMA4GA1Ud\n"
"DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAOPeoKiROOf7tFF4s8HerMxEJ\n"
"XdoiWCB1NwOX3Y0DUWDZ5JTbe3xouw6ANWiyFpDfY7UNLOoxVfLll97KsBTeLsRQ\n"
"EjIZ7ZzyzNv/2zb1dDEsOkJ8piN1eJLJQBiZhID7YoMJQytQSHR9a9M+nv0sE5vE\n"
"HEWhIJ/1c7FrLjJjZhm2MySbIbGmmi1hrOt0sp+/rh/coxSu0xkHNJVu2XBKAFYo\n"
"1lNk8E+6uZdpwVmvCpWq3Og5zBFtLxSXo6U+HNNjZ4Uk7J+O77n5yp26Fc8Ee8Bi\n"
"79tODcSf76pUyhz1/zmhR08voacZSyL/ijuMkBHicJpKtJDeklubukKyMQ5EMg==\n"
"-----END CERTIFICATE-----\n";

const char* privateKey = "-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEowIBAAKCAQEApjOQf6C22Mn+82UDnzPDZN5xA1An9ONSyX8JaNQGiC3LR0mi\n"
"oiC4m5yQ7/bfW0B6gRSAL+wildjBYafRRx0GFTUyEfzA0rasC9KhXW+qjOz8erbG\n"
"M/G5JwXgXZ/d3E8iOJGmvnFKdxn2LWMbR8m002bTFYMWTog4D3uaUfE0q+DQA987\n"
"uiD290T2h+kfcliIBriaTihRJp2J2VwXjmfBnyWDrsLlAnpeyOa9LzWoxuHvArKk\n"
"vy450GAcbjWI2fiEW6nDHe1VxgLMs3P4urdJgCuUFtG1vmyP/Kwkme0Ccwjxqy+S\n"
"QlqXNCX2nlq2w+KKkxMpVlr1tiIkg/JHmtEAbQIDAQABAoIBAGBtFE3n6Y3XjBKc\n"
"rcBFIP05zWm1rKj45sQ+7y+hq/SAVSsT/sZ/o3utF64Rlj70Jt8TDNhA8UPtES1e\n"
"Vx4iKbBOP7GN6jWWhDULzFFQgyvmJwnvVwk2Gk0YrLbFFMxIITuezZY6VpDJQDnP\n"
"ATniPgZU5uzZNCz1/UiGVt9pyTxacBeNRwlbGSo0rBtO9C/Gsu7GwDnpcYZZk24Q\n"
"qZQ6ZtrCStPe+tHsOR4jpyKT+4ArjF5Ws5FY5xdHwyUrieLJYtmSwm914lFNRqKA\n"
"bUoH+BWeyMujy2WrztUreDD7usVasaf9auB2btSf02t6e41DDZrZ6dze7CRePrrC\n"
"0+ZCGWUCgYEA0LoMK6aKysGOy75ZzebsGiL0jyS82t1vq9dYY26K1WtM5G7+rezq\n"
"Qho2Do2HYg89MAzBflFnvU8iq/wvEc2P5DhvzUs2xmJjJuxs+012mB+BONy3+3Sa\n"
"EMHAu1voecKn2w7rq9NrRlSDYEgpu+l5qftCRkpXfPCHBlu9sanmF6sCgYEAy9fn\n"
"T524zXNnsBpduL1T3QR+87GSb2Lbdap2AF+wksiXm0TYIf26iPFdxeRHz0pe0pE5\n"
"3xFg31qmFXIfI+dBZWooowPrt4BxTm2TSevfxh+/vlnCvy/KwDw0GHrLgJh7RaCg\n"
"qIoxl1865v/6mWC83pZZ+YW5Jg7SeVUHRsbRUEcCgYEAjPzes61vGr3boqwVJQvS\n"
"d2UdJIOAPaZNhBmVzrQge0BbVAHVbc4V/kiOMw5OYGJCxm4XVSfPlwQZO2rAsJnV\n"
"lCa6/kTL3v0eVHBmvOj6Lo85Thto+yUr3B3D/cp7IaPmlgViE1Jfp8X2Z9bWueNf\n"
"Fv5tKeWVawNdcWIOpC9LCLkCgYArCbm30JgtIJmaz+ZL1AV527ImJ67AP8IQNN5X\n"
"M9znEN0Zlk5/1r9knHltB8a1mIEuuiTa2SN35P699mtmvDPNgr1ztac2BjNmGLKJ\n"
"X/jOLvPf77mzfhazwcJKDNscmpUkXUaubwcLa7rW/OLfMVKPOHVkvnwZ8fU0UuSH\n"
"aADXjwKBgDZW22vNKVUrT9JAERLjvZn5q9Eohq/vRzEDSD9fklkWHBHD03au+44R\n"
"D8e2W/9J0ERvzs/xlzB7o5UBahqbr+0P4EV4Y+w7Bnr15DQj0mSzaPJ2V2nHJJpQ\n"
"SsX6MiuRIWSjJbtlCzxOjGGRKE3ASPuW2hjmi/NfYqpaKopQFIni\n"
"-----END RSA PRIVATE KEY-----\n";

const char ip[] = "10.1.10.253";
const char url[] = "ioth2o.com";
const String APssid = APSSID;
const String APpassword = "";
String UnHtmlForm = "";
int MemResetPin = 13;
String form = "";
String ssidWifi;
String passwordWifi;

/*****************************************************************************************
 ********** The following functions are based on memory storage and retrevial*************
 ****************************************************************************************/
 
String memRead(int l, int p) {
  String temp;
  for (int n = p; n < l + p; ++n)
  {
    if (char(EEPROM.read(n)) != ';') {
      if (isWhitespace(char(EEPROM.read(n)))) {

      } else temp += String(char(EEPROM.read(n)));

    } else n = l + p;

  }
  return temp;
}

void memWrite(String s, String p) {
  s += ";";
  writeEEPROM(s, 10);
  p += ";";
  writeEEPROM(p, 110);
  EEPROM.commit();
}

void writeEEPROM(String x, int pos) {
  for (int n = pos; n < x.length() + pos; n++) {
    EEPROM.write(n, x[n - pos]);
  }
}

boolean validSSID(String S) {
  if (S.charAt(0) > 32 && S.charAt(0) < 122) {
    return true;
  } else {
    return false;
  }
}

void memClear(String s, String p) {
  String clearS = "";
  String clearP = "";
  for (int i = 0; i < s.length(); i++) {
    clearS += (char)0;
  }
  for (int i = 0; i < p.length(); i++) {
    clearP += (char)0;
  }
  memWrite(clearS, clearP);
}

/*****************************************************************************************
 *************** The following functions are based on network function *******************
 ****************************************************************************************/

void networkSearchPrint() {
  Serial.print("Scan start ... ");
  int n = WiFi.scanNetworks();
  Serial.println(n);
  Serial.println(" network(s) found");
  for (int i = 0; i < n; i++) {
    Serial.println(i);
    Serial.println(WiFi.SSID(i));
  }
  Serial.println();
}

String htmlForm() {                       //generates the html for the network config webpage (includes the scaning and inclusion of new local network options)
  String htmlForm = "<!DOCTYPE HTML>";
  htmlForm += "<html>";
  htmlForm += "<head>";
  htmlForm += "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">";
  htmlForm += "<title>ESP8266 Web Form Demo</title>";
  htmlForm += "<style>";
  htmlForm += "\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\"";
  htmlForm += "</style>";
  htmlForm += "</head>";
  htmlForm += "<body>";
  htmlForm += "<h1>Enter Data Here</h1>";
  htmlForm += "<FORM action=\"/\" method=\"post\">";
  htmlForm += "<P>";
  htmlForm += "<select required name=\"ssid\">";
  htmlForm += "<option value=\"\">None</option>";
  int c = 4;
  String ssidList[45];              //holds the ssid values of local networks (max is 46 ssids)
  for (int i = 0; i < 3; i++) {     //scans the local avavible networks three times
    c = WiFi.scanNetworks();
    for (int j = 0; j < c; j++) {
      ssidList[j] = WiFi.SSID(j);
    }
  }
  for (int i = 0; i < 45; i++) {
    for (int j = i + 1; j < 45; j++) {
      if (ssidList[j] == ssidList[i]) {
        ssidList[j] = "";
      }
    }
  }
  for (int i = 0; i < 45; i++) {
    Serial.println(ssidList[i]);
  }
  for (int i = 0; i < 45; i++) {
    if (ssidList[i] == "") continue;
    htmlForm += "<option value=\"";
    htmlForm += ssidList[i];
    htmlForm += "\">";
    htmlForm += ssidList[i];
    htmlForm += "</option>";
  }
  htmlForm += "</select><BR>";
  htmlForm += "<INPUT type=\"text\" name=\"password\" placeholder=\"password\"><BR>";
  htmlForm += "<INPUT type=\"submit\" value=\"Send\"> <INPUT type=\"button\" name=\"reset\" value=\"reset\">";
  Serial.println("Reformat");
  htmlForm += "</P>";
  htmlForm += "</FORM>";
  htmlForm += "</body>";
  htmlForm += "</html>";
  return htmlForm;
}

boolean handleSubmit() {
  String ssidWifi = server.arg("ssid");
  String passwordWifi = server.arg("password");
  if (validSSID(ssidWifi)) {
    if (wifiConnect(ssidWifi, passwordWifi)) {
      WiFi.softAPdisconnect(true);
      memWrite(ssidWifi, passwordWifi);
      Serial.print("ssid:");
      Serial.println(ssidWifi);
      Serial.print("password:");
      Serial.println(passwordWifi);
      return false;
    }
  }
  return true;
}

//Refreshes the http server if data has not been input and sent
void handleRoot() {
  if (server.hasArg("ssid")) {
    handleSubmit();
  }
  else {
    server.send(200, "text/html", form);
  }
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void softAPConnect() {
  char APssidChar[APssid.length()+1];
  char APpasswordChar[APpassword.length()+1];
  APssid.toCharArray(APssidChar, APssid.length()+1);
  APpassword.toCharArray(APpasswordChar, APpassword.length()+1);
  
  WiFi.softAP(APssidChar, APpasswordChar);
  Serial.println("");
  Serial.print("Hosting: ");
  Serial.println(APssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
}

boolean wifiConnect(String s, String p) {
  WiFi.persistent(false);

  char ssidChar[s.length()+1];
  char passChar[p.length()+1];
  s.toCharArray(ssidChar, s.length()+1);
  p.toCharArray(passChar, p.length()+1);

  Serial.println(s);
  Serial.println(p);
  
  WiFi.begin(ssidChar, passChar);
  Serial.println("");
  int i = 0;
  delay(10);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    i++;
    if (i == 300) {
      WiFi.disconnect();
      Serial.println("Wifi Connection Failed, Starting AP");
      return false;
    }
  }
  return true;
}

/*****************************************************************************************
 ****** The following functions are based on data retrieval and RF communcations *********
 ****************************************************************************************/
 
String ICRequestData() {
  ESPserial.begin(9600);                  //starts the serial com line between the ESP and the NS7000 (9600 baudrate)
  ESPserial.write(65);                    //value taht the Pic is waiting to see before sending the data.
  Serial.println("\nWrite");
  byte bytesRead = 0;
  byte data[16];
  delay(100);
  noInterrupts();                         //Disable and other process that may negtavilty affect the abilty to read. 
  while (ESPserial.available() > 0) {
    data[bytesRead] = ESPserial.read();
    bytesRead++;
  }
  interrupts();                         //Reenable the interrupt operations as the timesinsitivity of operatios as decreased. 

  for (int a = 0; a < 16; a++) {
    Serial.print(data[a], DEC);
    if (a != 15) {
      Serial.print("~");
    }
  }
  Serial.println("\n");
  ESPserial.end();                        //Closing of the serial com line between the esp and the ns7000 in an attepmt to lower power draw
  return String((char *)data);
}

void dataSend(String d) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonArray& rxpk = root.createNestedArray("rxpk");
  JsonObject& data = rxpk.createNestedObject();
  data["data"] = d;

  WiFi.printDiag(Serial);

  String f1 = "";
  root.printTo(f1);
  char f2[f1.length()];
  f1.toCharArray(f2, f1.length() + 1);
  Serial.print(f1);
  
  connectAWSIoT();
  mqttClient.publish(pubTopic, f2);
}

void connectAWSIoT() {
    while (!mqttClient.connected()) {
        if (mqttClient.connect("ESP32_device")) {
            Serial.println("Connected.");
        } else {
            Serial.print("Failed. Error state=");
            Serial.print(mqttClient.state());
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void mqttCallback (char* topic, byte* payload, unsigned int length) {
    Serial.print("Received. topic=");
    Serial.println(topic);
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.print("\n");
}

/*****************************************************************************************
 ****************** Device setup and operation management functions **********************
 ****************************************************************************************/


void deviceConfig() {
  //WiFi.forceSleepWake();
  delay(1);
  WiFi.mode( WIFI_STA );
  form = htmlForm();                      //holds webpage html
  networkSearchPrint();
  softAPConnect();                        //sets up a WiFi AP
  while (handleSubmit()) {
    delay(25);
    server.handleClient();
  }
  Serial.println("Setup Complete");
  WiFi.mode( WIFI_OFF );                    //Disable the network part of the esp to allow lower power consumption
  //WiFi.forceSleepBegin();
  delay(3);
}

void setup(void) {
  WiFi.mode( WIFI_OFF );
  //WiFi.forceSleepBegin();
  delay(1);
  Serial.begin(115200);
  EEPROM.begin(512);
  pinMode(MemResetPin, INPUT);
  ssidWifi = memRead(30, 10);
  passwordWifi = memRead(30, 110);
  
  // Configure MQTT Client
  httpsClient.setCACert(rootCA);
  httpsClient.setCertificate(certificate);
  httpsClient.setPrivateKey(privateKey);
  mqttClient.setServer(endpoint, port);
  mqttClient.setCallback(mqttCallback);
    
  if (validSSID(ssidWifi) == false) {
    deviceConfig();
  }
}

void loop(void) {
  if (digitalRead(MemResetPin) == 1) {
    Serial.println("1");
    memClear(ssidWifi, passwordWifi);       //Overwrite the ESP Flash where SSID and Password is stored 
    Serial.println("2");
    ESP.restart();                          //Restart the ESP
    Serial.println("3");
  }
  Serial.println("4");
  String data = ICRequestData();            //Sets data to the data sent from the NS7000
  Serial.println("5");
  Serial.println(data);
  Serial.println("6");
  Serial.println(ssidWifi);
  Serial.println(passwordWifi);
  if (data.charAt(15) == 255) {             //Check if the ending byte is correct therefore confirming the data is atleast structured correct
    //WiFi.forceSleepWake();                  //wake up the networking capibilites of the ESP
    delay(1);
    WiFi.mode( WIFI_STA );                  //standard wifi config
    if (wifiConnect(ssidWifi, passwordWifi)) {
      dataSend(data);
      WiFi.disconnect();
    }
    WiFi.mode( WIFI_OFF );                  //disable wifi config
    //WiFi.forceSleepBegin();                 //put networking capibilites of the esp to sleep
    delay(1);
  }
  else {
    Serial.println("Data Collect Fail or Network Connection Failure");
  }
  delay(100);
}
