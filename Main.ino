#include <stddef.h>
#include <stdlib.h>
#include "WiFi.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


//Const values
#define NTP_OFFSET  28800 // In seconds 
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "1.asia.pool.ntp.org"
const String etherLink = "https://trade.cointree.com/api/prices/AUD/change/24h?symbols=ETH";
const String btcLink = "https://trade.cointree.com/api/prices/AUD/change/24h?symbols=BTC";
const String timeLink = "http://worldtimeapi.org/api/ip";
const char* ssid = "WIFI AP";
const char* password =  "PASSWORD";
const String openWeatherMapApiKey = "API_KEY";
const String city = "Perth";
const String countryCode = "AU";
const int contrastValue = 60; // Default Contrast Value
unsigned long weatherTimer = 0; //Be careful to change, will get blocked if too many requests are made
unsigned long cryptoTimer = 0;

//Display Config
Adafruit_PCD8544 display = Adafruit_PCD8544(18, 23, 4, 15, 2);
Adafruit_PCD8544 display2 = Adafruit_PCD8544(18, 23, 4, 13, 14);
Adafruit_PCD8544 display3 = Adafruit_PCD8544(18, 23, 4, 12, 14);

//Time Config
WiFiUDP ntpUDP; 
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

unsigned long lastTime = 0; //Last time of http request

//Weather variables
float temp = 0;
float humidity = 0;

//Crypto variables
float btc = 0;
float ether = 0;

/**
 * Make an http request
 */
String httpGETRequest(const char* serverName) {
  HTTPClient http;
  // Your IP address with path or Domain name with URL path 
  http.begin(serverName);
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  
  // Free resources
  http.end();

  return payload;
}

/**
 * Init method
 */
void setup()
{
  
  // Init WiFi
  Serial.begin(115200);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");
  
  timeClient.begin();

  // Initialize the Display
  display.begin();
  display2.begin();
  display3.begin();

  // Change the displays contrast values
  display.setContrast(75);
  display2.setContrast(65);
  display3.setContrast(65);

  //Set the rotation
  display.setRotation(2);
  display2.setRotation(2);
  display3.setRotation(2);
  
  // Clear the displays buffers
  display.clearDisplay();
  display.display();
  display2.clearDisplay();
  display2.display();
  display3.clearDisplay();
  display3.display();


  //Add a delay for so the displays have some time to init
  delay(1000);
  
}

void getWeather() {
  StaticJsonDocument<2000> doc;
  // Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED){
    String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;
    String strResponse = httpGETRequest(serverPath.c_str());
    Serial.println(strResponse);
    DeserializationError error = deserializeJson(doc, strResponse, DeserializationOption::NestingLimit(10));

    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    float tempVal = doc["main"]["temp"];
    temp = tempVal - 273.15;
    humidity = doc["main"]["humidity"];
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}

void getEther() {
  StaticJsonDocument<2000> doc;
  // Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED){
    String strResponse = httpGETRequest(etherLink.c_str());
    Serial.println(strResponse);
    DeserializationError error = deserializeJson(doc, strResponse, DeserializationOption::NestingLimit(10));

    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    
    ether = doc[0]["currentRate"];
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}


void getBTC() {
  StaticJsonDocument<2000> doc;
  // Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED){
    String strResponse = httpGETRequest(btcLink.c_str());
    Serial.println(strResponse);
    DeserializationError error = deserializeJson(doc, strResponse, DeserializationOption::NestingLimit(10));

    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    
    btc = doc[0]["currentRate"];
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}


void getTime() {

}

void loop()
{
  timeClient.update();
  Serial.print("Time: ");
  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.println(timeClient.getMinutes());

  //Display 1 test
  display.clearDisplay();
  // Now let us display some text
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.setCursor(3,0);
  display.println("-- Weather --");
  display.setCursor(15,20);
  display.print("T: ");
  display.print(temp);
  display.println("c");
  display.setCursor(15,32);
  display.print("H: ");
  display.print(humidity);
  display.println("%");
  display.display();

  delay(500);

  display2.clearDisplay();
  display2.setTextColor(BLACK);
  display2.setTextSize(1);
  display2.setCursor(6,0);
  display2.println("-- Crypto --");
  display2.setCursor(3,20);
  display2.print("ETH: ");
  display2.println(ether);
  display2.setCursor(3,32);
  display2.print("BTC: ");
  display2.println(btc);
  display2.display();

  delay(500);
  display3.clearDisplay();
  display3.setTextColor(BLACK);
  display3.setTextSize(1);
  display3.setCursor(12,0);
  display3.println("-- Time --");
  display3.setCursor(27,20);
  if (timeClient.getHours() < 10) {
    display3.print("0");
  }
  display3.print(timeClient.getHours());
  display3.print(":");
  if (timeClient.getMinutes() < 10) {
    display3.print("0");
  }
  display3.println(timeClient.getMinutes());
  display3.display();

  delay(500);

  if ((millis() - lastTime) > cryptoTimer) {
    Serial.println("Getting crypto.");
    getEther();
    getBTC();

    Serial.print("ETH: ");
    Serial.println(ether);
    Serial.print("BTC: ");
    Serial.println(btc);

    cryptoTimer = 100000;
  }
  
  // Time required to wait till next refresh, if this is too high or low
  // then we are going to be writing to the display to quick and nothing will show
  if ((millis() - lastTime) > weatherTimer) {
    Serial.println("Getting Weather.");
    getWeather();
    Serial.print("Temp: ");
    Serial.println(temp);
    Serial.print("Humidity: ");
    Serial.println(humidity);

    weatherTimer = 3600000;
    lastTime = millis();
  }
}
