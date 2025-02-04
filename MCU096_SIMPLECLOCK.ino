#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>

// Replace with your network credentials
const char* ssid = "XXXXXXXXXXXXXXXXXXXX";
const char* password = "XXXXXXXXXXXXXXXX";

// OLED display settings
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 12, /* data=*/ 14);

unsigned long previousMillis = 0;
const long interval = 3600000; // 60 minutes in milliseconds

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");

  // Initialize the OLED display
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.sendBuffer();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected to Wi-Fi");
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Fetch Bitcoin price
    String bitcoinPrice = getBitcoinPrice();

    // Display the price on the OLED screen
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);  // Display font
    u8g2.drawStr(0, 30, "BTC Price:");   // Position
    u8g2.drawStr(0, 50, bitcoinPrice.c_str());  // Position
    u8g2.sendBuffer();
  }
}

String getBitcoinPrice() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://api.coindesk.com/v1/bpi/currentprice/BTC.json"); // API endpoint
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      StaticJsonDocument<1024> doc;
      deserializeJson(doc, payload);
      float price = doc["bpi"]["USD"]["rate_float"];
      http.end();
      return String(price, 2); // Return price with 2 decimal places
    } else {
      Serial.println("Error on HTTP request");
      http.end();
      return "Error";
    }
  } else {
    return "No WiFi";
  }
}