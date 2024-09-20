#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

RTC_DS3231 rtc;
DateTime now;

#define BUTTON_PIN D0

// WiFi CREDENTIALS HERE
const char* ssid = "";
const char* password = "";

// IST Timezone
const char* timeServer = "http://worldtimeapi.org/api/timezone/Asia/Kolkata";

bool buttonPressed = false;
unsigned long wifiRetryTimeout = 10000;

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

  if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
    buttonPressed = true;

    if (WiFi.status() == WL_CONNECTED) {
      updateTimeFromCloud();
    } 
    else {
      Serial.println("Wi-Fi not connected, attempting to reconnect...");
      if (attemptReconnect()) {
        Serial.println("Reconnected! Updating time from cloud...");
        updateTimeFromCloud();
      } else {
        Serial.println("Failed to reconnect. Continuing with current RTC time.");
      }
    }
  }

  if (digitalRead(BUTTON_PIN) == HIGH) {
    buttonPressed = false;
  }

  delay(1000);
}

void displayTime(DateTime currentTime) {
  display.clearDisplay();

  int hour = currentTime.hour();
  String period = "AM";

  if (hour == 0) {
    hour = 12;
  } else if (hour == 12) {
    period = "PM";
  } else if (hour > 12) {
    hour -= 12;
    period = "PM";
  }

  display.setTextSize(4);

  int xPos = (SCREEN_WIDTH - (24 * 5)) / 2;
  int yPos = (SCREEN_HEIGHT - 32) / 2;

  display.setCursor(xPos, yPos);
  display.printf("%02d:%02d", hour, currentTime.minute());

  if (period == "PM") {
    display.fillCircle(SCREEN_WIDTH - 5, SCREEN_HEIGHT - 5, 2, WHITE);
  }

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

      const char* datetime = doc["datetime"];
      Serial.println(datetime);

      int year, month, day, hour, minute, second;
      sscanf(datetime, "%4d-%2d-%2dT%2d:%2d:%2d", &year, &month, &day, &hour, &minute, &second);

      rtc.adjust(DateTime(year, month, day, hour, minute, second));
      Serial.println("RTC time updated from cloud.");

    } else {
      Serial.println("Failed to fetch time from the server.");
    }

    http.end();
  } else {
    Serial.println("Not connected to Wi-Fi.");
  }
}

void connectToWiFi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi Connected.");
}

bool attemptReconnect() {
  unsigned long startTime = millis();
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED && millis() - startTime < wifiRetryTimeout) {
    delay(500);
    Serial.print(".");
  }

  return WiFi.status() == WL_CONNECTED;
}
