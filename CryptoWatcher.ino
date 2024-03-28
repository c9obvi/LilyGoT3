#include <WiFi.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h> // Include ArduinoJson library

// WiFi credentials
const char* ssid1 = "wifiAP3";
const char* password1 = "edpo1-Byfkar";
const char* ssid2 = "Mro-engWifi";
const char* password2 = "mRomRomRo1";

// Access Point settings
const char* apSSID = "BitcoinDisplay";
const char* apPassword = "21000000";

// Array of cryptocurrency IDs for CoinGecko API
const char* cryptoIds[] = {"bitcoin", "ethereum", "dogecoin", "monero"};
const char* cryptoNames[] = {"BTC", "ETH", "DOGE", "XMR"}; // User-friendly names for display
int currentCryptoIndex = 0; // Index to track the current cryptocurrency
int numCryptos = sizeof(cryptoIds) / sizeof(cryptoIds[0]); // Number of cryptocurrencies

const int buttonPin = 14; // Example pin, adjust to your actual setup

// TFT display
TFT_eSPI tft = TFT_eSPI();

unsigned long previousMillis = 0; // Stores the last time the update was made
const long interval = 50000; // Interval at which to refresh (milliseconds, 50 seconds)

void setupWiFi() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Crypto Watch", 10, 10, 4);
  Serial.println("Connecting to WiFi...");
  
  WiFi.begin(ssid1, password1);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
    if(attempts == 10) {
      Serial.println("Trying second WiFi network...");
      WiFi.begin(ssid2, password2);
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi");
    tft.fillScreen(TFT_BLACK);
    tft.drawString("Connected to WiFi", 10, 10, 4);
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    tft.drawString("IP: " + WiFi.localIP().toString(), 10, 30, 4);
  } else {
    Serial.println("\nFailed to connect to WiFi. Setting up AP...");
    WiFi.softAP(apSSID, apPassword);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
  }
}

void fetchCryptoData(float &price, float &percentChange, String cryptoId) {
  HTTPClient http;
  String requestURL = "https://api.coingecko.com/api/v3/simple/price?ids=" + cryptoId + "&vs_currencies=usd&include_24hr_change=true";
  http.begin(requestURL);
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println(httpCode);
    Serial.println(payload);

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    price = doc[cryptoId]["usd"].as<float>();
    percentChange = doc[cryptoId]["usd_24h_change"].as<float>();
  } else {
    Serial.println("Error on HTTP request");
  }

  http.end();
}

void displayCryptoData(float price, float percentChange, const char* cryptoName) {
    tft.fillScreen(TFT_BLACK); // Clear the screen
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(String(cryptoName) + " Price: $" + String(price), 10, 20, 4);
    tft.drawString("24Hr Change: " + String(percentChange) + "%", 10, 50, 4);
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  setupWiFi();
  pinMode(buttonPin, INPUT_PULLUP); // Initialize the button pin as input with pull-up resistor
  
  // Display initial data for the first cryptocurrency
  float price = 0, percentChange = 0;
  fetchCryptoData(price, percentChange, cryptoIds[currentCryptoIndex]);
  displayCryptoData(price, percentChange, cryptoNames[currentCryptoIndex]);
}

void loop() {
  unsigned long currentMillis = millis();
  static unsigned long lastButtonPress = 0; // Timestamp of the last button press
  
  // Debounce and detect button press
  if (digitalRead(buttonPin) == LOW && millis() - lastButtonPress > 200) { // 200ms debounce
    lastButtonPress = millis(); // Update last button press time
    currentCryptoIndex = (currentCryptoIndex + 1) % numCryptos; // Cycle to the next cryptocurrency

    // Fetch and display new cryptocurrency data immediately
    float price = 0, percentChange = 0;
    fetchCryptoData(price, percentChange, cryptoIds[currentCryptoIndex]);
    displayCryptoData(price, percentChange, cryptoNames[currentCryptoIndex]);
  }
  
  // Regular data refresh logic
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    float price = 0, percentChange = 0;
    fetchCryptoData(price, percentChange, cryptoIds[currentCryptoIndex]);
    displayCryptoData(price, percentChange, cryptoNames[currentCryptoIndex]);
  }
}
