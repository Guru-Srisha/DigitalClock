#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define BUTTON_PIN 2
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
RTC_DS3231 rtc;
DateTime now;

//Wi-Fi credentials
const char* ssid = "";
const char* password = "";

//API key
const String apiKey = "";
const String timeServer = "http://api.timezonedb.com/v2.1/get-time-zone?key=" + apiKey + "";

bool buttonPressed = false;
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 60000;

void setup() {
  Serial.begin(115200);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  connectToWiFi();
}

void loop() {
  now = rtc.now();
  displayTime(now);
  if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed && WiFi.status() == WL_CONNECTED) {
    buttonPressed = true;
    unsigned long currentTime = millis();
    if (currentTime - lastUpdate >= updateInterval) {
      updateTimeFromCloud();
      lastUpdate = currentTime;
      display.clearDisplay();
      display.setTextSize(6);
      int xPos = (SCREEN_WIDTH - 36 * 2) / 2;
      int yPos = (SCREEN_HEIGHT - 48) / 2;
      display.setCursor(xPos, yPos);
      display.print(":D");
      display.display();
      delay(1000);
    }
  }
  if (digitalRead(BUTTON_PIN) == HIGH) buttonPressed = false;
  delay(1000);
}

void displayTime(DateTime currentTime) {
  display.clearDisplay();
  int hour = currentTime.hour();
  String period = "AM";
  if (hour == 0) hour = 12;
  else if (hour == 12) period = "PM";
  else if (hour > 12) {
    hour -= 12;
    period = "PM";
  }
  display.setTextSize(4);
  int xPos = (SCREEN_WIDTH - (24 * 5)) / 2;
  int yPos = (SCREEN_HEIGHT - 32) / 2;
  display.setCursor(xPos, yPos);
  display.printf("%02d:%02d", hour, currentTime.minute());
  if (period == "PM") display.fillCircle(SCREEN_WIDTH - 5, SCREEN_HEIGHT - 5, 2, WHITE);
  display.display();
}

void updateTimeFromCloud() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    http.begin(client, timeServer);
    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);
      const char* formattedTime = doc["formatted"];
      Serial.println(formattedTime);
      int year, month, day, hour, minute, second;
      sscanf(formattedTime, "%4d-%2d-%2d %2d:%2d:%2d", &year, &month, &day, &hour, &minute, &second);
      rtc.adjust(DateTime(year, month, day, hour, minute, second));
      Serial.println("RTC time updated from cloud");
    } else Serial.println("Failed to fetch time from the server");
    http.end();
  } else Serial.println("Not connected to WiFi");
}

void connectToWiFi() {
  display.clearDisplay();
  display.setTextSize(5);
  display.setTextColor(WHITE);
  int xPos = (SCREEN_WIDTH - (30 * 3)) / 2;
  int yPos = (SCREEN_HEIGHT - 40) / 2;
  display.setCursor(xPos, yPos);
  display.println("?_?");
  display.display();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  display.clearDisplay();
  xPos = (SCREEN_WIDTH - (30 * 3)) / 2;
  yPos = (SCREEN_HEIGHT - 40) / 2;
  display.setCursor(xPos, yPos);
  display.println("!_!");
  display.display();
  delay(1000);
}
