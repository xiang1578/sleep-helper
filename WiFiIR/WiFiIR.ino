#include <M5Stack.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include "WebServer.h"
#include <Preferences.h>
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <Adafruit_NeoPixel.h>
#include <stdio.h>
#include "DHT12.h"
#include <Wire.h> //The DHT12 uses I2C comunication.
#include "Adafruit_Sensor.h"
#include <Adafruit_BMP280.h>

#define RGBPIN 15
#define PIXELNUM 10
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXELNUM, RGBPIN, NEO_GRB + NEO_KHZ800);

const IPAddress apIP(192, 168, 4, 1);
const char* apSSID = "M5STACK_SETUP";
boolean settingMode;
String ssidList;
String wifi_ssid;
String wifi_password;

boolean autoMode;
int autoState;

const uint16_t kIrLed = 26;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.
uint16_t powerOnData[73] = {9000, 4500,  670, 540,  670, 540,  670, 540,  
                            670, 1600,  670, 540,  670, 540,  670, 540,  
                            670, 540,  670, 1600,  670, 540,  670, 540,  
                            670, 1600,  670, 540,  670, 540,  670, 540,  
                            670, 540,  670, 540,  670, 540,  670, 540,  
                            670, 540,  670, 540,  670, 540,  670, 540,  
                            670, 540,  670, 540,  670, 540,  670, 540,  
                            670, 540,  670, 1600,  670, 540,  670, 1600,  
                            670, 540,  670, 540,  670, 1600,  670, 540,  670};
uint16_t powerOffData[73] = {9000, 4500,  670, 540,  670, 540,  670, 540,  
                              670, 540,  670, 540,  670, 540,  670, 540,  
                              670, 540,  670, 540,  670, 540,  670, 540,  
                              670, 1600,  670, 540,  670, 540,  670, 540,  
                              670, 540,  670, 540,  670, 540,  670, 540,  
                              670, 540,  670, 540,  670, 540,  670, 540,  
                              670, 540,  670, 540,  670, 540,  670, 540,  
                              670, 540, 670, 1600,  670, 540,  670, 1600,  
                              670, 540,  670, 540,  670, 1600,  670, 540,  670};

DHT12 dht12; //Preset scale CELSIUS and ID 0x5c.
Adafruit_BMP280 bme;

// DNSServer dnsServer;
WebServer webServer(80);

// wifi config store
Preferences preferences;

void setup() {
  M5.begin();
  irsend.begin();
  pixels.begin();
  Wire.begin();
  preferences.begin("wifi-config");

  M5.Power.begin();
#if ESP8266
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
#else  // ESP8266
  Serial.begin(115200, SERIAL_8N1);
#endif  // ESP8266

  delay(10);
  if (restoreConfig()) {
    if (checkConnection()) {
      settingMode = false;
      startWebServer();
      return;
    }
  }
  settingMode = true;
  autoMode = true;
  autoState = 0;
  setupMode();
}

int loopCount = 0;

void loop() {
  if (!settingMode && loopCount > 5000) {
    if(autoMode && bme.begin(0x76)) {
      loopCount = 0;
      if (autoState != 2 && dht12.readTemperature() > 27) {
        irsend.sendRaw(powerOnData, 73, 38);
        autoState = 2;
      }
      if (autoState != 1 && dht12.readTemperature() < 25) {
        irsend.sendRaw(powerOffData, 73, 38);
        autoState = 1;
      }
      M5.Lcd.fillRect(0, 120, 160, 100, WHITE);
      M5.Lcd.setCursor(40, 120);
      M5.Lcd.setTextColor(BLACK);
      M5.Lcd.setTextSize(4);
      M5.Lcd.printf("%d C\n", (int)(dht12.readTemperature()));
      M5.Lcd.setTextSize(2);
      M5.Lcd.printf(" \n   Auto Mode");
    }
    else {
      M5.Lcd.fillRect(0, 120, 160, 100, WHITE);
    }
  }
  webServer.handleClient();
  loopCount += 1;
}

boolean restoreConfig() {
  wifi_ssid = preferences.getString("WIFI_SSID");
  wifi_password = preferences.getString("WIFI_PASSWD");
  /*Serial.print("WIFI-SSID: ");
  M5.Lcd.print("WIFI-SSID: ");
  Serial.println(wifi_ssid);
  M5.Lcd.println(wifi_ssid);
  Serial.print("WIFI-PASSWD: ");
  M5.Lcd.print("WIFI-PASSWD: ");
  Serial.println(wifi_password);
  M5.Lcd.println(wifi_password);*/
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

  if(wifi_ssid.length() > 0) {
    return true;
} else {
    return false;
  }
}

boolean checkConnection() {
  int count = 0;
  Serial.print("Waiting for Wi-Fi connection");
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(50, 100);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(3);
  M5.Lcd.printf("Connecting...");
  //M5.Lcd.print("Waiting for Wi-Fi connection");
  while ( count < 30 ) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      //M5.Lcd.println();
      Serial.println("Connected!");
      //M5.Lcd.println("Connected!");
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(50, 100);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setTextSize(3);
      M5.Lcd.printf("Connected!");
      return true;
    }
    delay(500);
    Serial.print(".");
    //M5.Lcd.print(".");
    M5.Lcd.fillRect(10, 200, 300, 40, BLACK);
    M5.Lcd.fillRect(10 + count * 5, 200, 300 - count * 10, 40, GREEN);
    count++;
  }
  Serial.println("Timed out.");
  //M5.Lcd.println("Timed out.");
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(50, 100);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(3);
  M5.Lcd.printf("Timed out.");
  return false;
}

void startWebServer() {
  if (settingMode) {
    Serial.print("Starting Web Server at ");
    //M5.Lcd.print("Starting Web Server at ");
    Serial.println(WiFi.softAPIP());
    //M5.Lcd.println(WiFi.softAPIP());
    webServer.on("/settings", []() {
      String s = "<h1 style=\"text-align: center\">Wi-Fi Settings</h1><hr>";
      s += "<form method=\"get\" action=\"setap\" style=\"text-align: center\"><label>SSID: </label><select name=\"ssid\">";
      s += ssidList;
      s += "</select><br><br>Password: <input name=\"pass\" length=64 type=\"password\"><input type=\"submit\"></form>";
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
    });
    webServer.on("/setap", []() {
      String ssid = urlDecode(webServer.arg("ssid"));
      Serial.print("SSID: ");
      //M5.Lcd.print("SSID: ");
      Serial.println(ssid);
      //M5.Lcd.println(ssid);
      String pass = urlDecode(webServer.arg("pass"));
      Serial.print("Password: ");
      //M5.Lcd.print("Password: ");
      Serial.println(pass);
      //M5.Lcd.println(pass);
      Serial.println("Writing SSID to EEPROM...");
      //M5.Lcd.println("Writing SSID to EEPROM...");

      // Store wifi config
      Serial.println("Writing Password to nvr...");
      //M5.Lcd.println("Writing Password to nvr...");
      preferences.putString("WIFI_SSID", ssid);
      preferences.putString("WIFI_PASSWD", pass);

      Serial.println("Write nvr done!");
      //M5.Lcd.println("Write nvr done!");
      String s = "<h1 style=\"text-align: center\">Setup complete!</h1 style=\"text-align: center\"><p style=\"text-align: center\">device will be connected to \"";
      s += ssid;
      s += "\" after the restart.";
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
      delay(3000);
      ESP.restart();
    });
    webServer.onNotFound([]() {
      String s = "<h1 style=\"text-align: center\">M5GO</h1><p style=\"text-align: center\"><a href=\"/settings\">Wi-Fi Settings</a></p>";
      webServer.send(200, "text/html", makePage("M5GO", s));
    });
  }
  else {
    Serial.print("Starting Web Server at ");
    //M5.Lcd.print("Starting Web Server at ");
    Serial.println(WiFi.localIP());
    //M5.Lcd.println(WiFi.localIP());
    M5.Lcd.fillScreen(WHITE);
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.setTextSize(3);
    M5.Lcd.printf(" Enjoy");
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("\n\n the Life.");
    M5.Lcd.qrcode(WiFi.localIP().toString(), 160, 80, 160, 6);
    webServer.on("/", []() {
      String s = "<h1 style=\"text-align: center\">Connected!</h1><h3 style=\"text-align: center\"><a href=\"/poweron\">Power On</a></h3><h3 style=\"text-align: center\"><a href=\"/poweroff\">Power Off</a></h3><h3 style=\"text-align: center\"><a href=\"/automode\">Auto Mode</a></h3><p style=\"text-align: center\"><a href=\"/reset\">Reset WiFi</a></p>";
      webServer.send(200, "text/html", makePage("M5GO", s));
    });
    webServer.on("/poweron", []() {
      irsend.sendRaw(powerOnData, 73, 38);
      for (int i = 0; i < PIXELNUM; i++) {
        pixels.setPixelColor(i, pixels.Color(0, 200, 0)); 
        pixels.show(); 
      }
      autoMode = false;
      autoState = 0;
      String s = "<meta http-equiv=\"refresh\" content=\"0;url=http://192.168.137.18/\">";
      webServer.send(200, "text/html", makePage("M5GO", s));
    });
    webServer.on("/poweroff", []() {
      irsend.sendRaw(powerOffData, 73, 38);
      for (int i = 0; i < PIXELNUM; i++) {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0)); 
        pixels.show(); 
      }
      autoMode = false;
      autoState = 0;
      String s = "<meta http-equiv=\"refresh\" content=\"0;url=http://192.168.137.18/\">";
      webServer.send(200, "text/html", makePage("M5GO", s));
    });
    webServer.on("/automode", []() {
      for (int i = 0; i < PIXELNUM; i++) {
        pixels.setPixelColor(i, pixels.Color(20, 20, 160)); 
        pixels.show(); 
      }
      autoMode = true;
      String s = "<meta http-equiv=\"refresh\" content=\"1;url=http://192.168.137.18/\">";
      webServer.send(200, "text/html", makePage("M5GO", s));
    });
    webServer.on("/reset", []() {
      // reset the wifi config
      preferences.remove("WIFI_SSID");
      preferences.remove("WIFI_PASSWD");
      String s = "<h1 style=\"text-align: center\">Wi-Fi settings was reset.</h1><p style=\"text-align: center\">device will restart.</p>";
      webServer.send(200, "text/html", makePage("Reset Wi-Fi Settings", s));
      delay(3000);
      ESP.restart();
    });
  }
  webServer.begin();
}

void setupMode() {
  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  delay(100);
  Serial.println("");
  //M5.Lcd.println("");
  for (int i = 0; i < n; ++i) {
    ssidList += "<option value=\"";
    ssidList += WiFi.SSID(i);
    ssidList += "\">";
    ssidList += WiFi.SSID(i);
    ssidList += "</option>";
  }
  delay(100);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID);
  WiFi.mode(WIFI_MODE_AP);
  // WiFi.softAPConfig(IPAddress local_ip, IPAddress gateway, IPAddress subnet);
  // WiFi.softAP(const char* ssid, const char* passphrase = NULL, int channel = 1, int ssid_hidden = 0);
  // dnsServer.start(53, "*", apIP);
  startWebServer();
  Serial.print("Starting Access Point at \"");
  //M5.Lcd.print("Starting Access Point at \"");
  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.setTextColor(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.printf(" Connect\n\n ");
  M5.Lcd.setTextSize(3);
  M5.Lcd.printf(apSSID);
  M5.Lcd.println("");
  M5.Lcd.setTextSize(2);
  M5.Lcd.printf("\n and Scan \n QRCode");
  Serial.print(apSSID);
  //M5.Lcd.print(apSSID);
  Serial.println("\"");
  //M5.Lcd.println("\"");
  M5.Lcd.qrcode(apIP.toString(), 160, 80, 160, 6);
}

String makePage(String title, String contents) {
  String s = "<!DOCTYPE html><html><head>";
  s += "<meta name=\"viewport\" content=\"width=device-width,user-scalable=0\">";
  s += "<title>";
  s += title;
  s += "</title></head><body>";
  s += contents;
  s += "</body></html>";
  return s;
}

String urlDecode(String input) {
  String s = input;
  s.replace("%20", " ");
  s.replace("+", " ");
  s.replace("%21", "!");
  s.replace("%22", "\"");
  s.replace("%23", "#");
  s.replace("%24", "$");
  s.replace("%25", "%");
  s.replace("%26", "&");
  s.replace("%27", "\'");
  s.replace("%28", "(");
  s.replace("%29", ")");
  s.replace("%30", "*");
  s.replace("%31", "+");
  s.replace("%2C", ",");
  s.replace("%2E", ".");
  s.replace("%2F", "/");
  s.replace("%2C", ",");
  s.replace("%3A", ":");
  s.replace("%3A", ";");
  s.replace("%3C", "<");
  s.replace("%3D", "=");
  s.replace("%3E", ">");
  s.replace("%3F", "?");
  s.replace("%40", "@");
  s.replace("%5B", "[");
  s.replace("%5C", "\\");
  s.replace("%5D", "]");
  s.replace("%5E", "^");
  s.replace("%5F", "-");
  s.replace("%60", "`");
  return s;
}
