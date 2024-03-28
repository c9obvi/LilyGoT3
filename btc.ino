#include <WiFi.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h> // Include ArduinoJson library

// WiFi credentials
const char* ssid1 = "*****";
const char* password1 = "*****";
const char* ssid2 = "*****";
const char* password2 = "*****";

// Fallback Access Point settings
const char* apSSID = "BitcoinDisplay";
const char* apPassword = "21000000";

// TFT display
TFT_eSPI tft = TFT_eSPI();

unsigned long previousMillis = 0; // Stores the last time the update was made
const long interval = 50000; // Interval at which to refresh (milliseconds, 50 seconds)

void setupWiFi() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Connecting to WiFi...", 10, 10, 2);
  Serial.println("Connecting to WiFi...");
  
  WiFi.begin(ssid1, password1);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    tft.drawString(".", 10 + (attempts * 6), 30, 2); // Display dots on screen
    attempts++;
    if(attempts == 10) {
      Serial.println("Trying second WiFi network...");
      tft.drawString("Trying 2nd WiFi...", 10, 50, 2);
      WiFi.begin(ssid2, password2);
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi");
    tft.fillScreen(TFT_BLACK);
    tft.drawString("Connected to WiFi", 10, 10, 2);
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    tft.drawString("IP: " + WiFi.localIP().toString(), 10, 30, 2);
  } else {
    Serial.println("\nFailed to connect to WiFi. Setting up AP...");
    tft.drawString("Setting up AP...", 10, 70, 2);
    WiFi.softAP(apSSID, apPassword);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
    tft.drawString("AP IP: " + WiFi.softAPIP().toString(), 10, 90, 2);
  }
}

void fetchBitcoinData(float &price, float &percentChange) {
  HTTPClient http;
  http.begin("https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd&include_market_cap=false&include_24hr_vol=false&include_24hr_change=true&include_last_updated_at=false");
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println(httpCode);
    Serial.println(payload);

    // Parse JSON to extract price and percentage change
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    price = doc["bitcoin"]["usd"].as<float>();
    percentChange = doc["bitcoin"]["usd_24h_change"].as<float>();
  } else {
    Serial.println("Error on HTTP request");
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  setupWiFi();
  
  float btcPrice = 0, percentChange = 0;
  fetchBitcoinData(btcPrice, percentChange);
  
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("BTC Price: $" + String(btcPrice), 10, 20, 4);
  tft.drawString("Change: " + String(percentChange) + "%", 10, 50, 4);
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Check if the interval time has passed (e.g., 15 seconds)
  if (currentMillis - previousMillis >= interval) {
    // It's time to update the Bitcoin data
    
    // Update previousMillis to the current time, preparing for the next interval
    previousMillis = currentMillis;

    float btcPrice = 0, percentChange = 0;
    fetchBitcoinData(btcPrice, percentChange);
    tft.fillScreen(TFT_BLACK); // Clear the screen
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("BTC Price: $" + String(btcPrice), 10, 20, 4);
    tft.drawString("Change: " + String(percentChange) + "%", 10, 50, 4);
  }
}

