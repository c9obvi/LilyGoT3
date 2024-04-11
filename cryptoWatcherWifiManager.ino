#include <WiFiManager.h> // Include the Wi-Fi Manager library
#include <TFT_eSPI.h>     // Graphics and font library for ST7735 driver chip
#include <ArduinoJson.h>  // Include ArduinoJson library for parsing the API response
#include <HTTPClient.h>

// Global objects and variables
TFT_eSPI tft = TFT_eSPI();   // Invoke library, pins defined in User_Setup.h
const char* cryptoIds[] = {"bitcoin", "ethereum", "dogecoin", "monero"};
const char* cryptoNames[] = {"BTC", "ETH", "DOGE", "XMR"};
int currentCryptoIndex = 0;
int numCryptos = sizeof(cryptoIds) / sizeof(cryptoIds[0]);
const int buttonPin = 14; // Pin for cycling through cryptocurrencies

unsigned long previousMillis = 0; // Stores the last time the update was made
const long interval = 60000; // Interval at which to refresh (milliseconds, 60 seconds)

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  
  // Display the initial splash screen
  //tft.setCursor(10, 10);
  //tft.println("CryptoWatcher");
  tft.drawString("Crypto Watcher", 10, 10, 4);
  //tft.setCursor(12, 50);
  //tft.println("By 0xBerto");
  tft.drawString("By 0xBerto", 12, 35, 4);
  tft.drawString("Configuring WiFi...", 10, 65, 4);
  delay(3000); // Show the splash screen for a few seconds

  // Initialize Wi-Fi using WiFiManager
  WiFiManager wifiManager;
  wifiManager.autoConnect("CryptoWatcherAP");
  tft.fillScreen(TFT_BLACK); // Clear the screen before proceeding
  Serial.println("Connected to Wi-Fi");

  // Display a connected message and IP address
  //tft.setCursor(10, 10);
  //tft.print("Connected! IP: ");
  //tft.println(WiFi.localIP());
  Serial.println("\nConnected to WiFi");
  tft.fillScreen(TFT_BLACK);
  tft.drawString("Connected to WiFi", 10, 10, 4);
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  delay(1000); // Display IP address briefly
  tft.drawString("IP: " + WiFi.localIP().toString(), 10, 80, 4);

  // Fetch and display initial cryptocurrency data
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

void fetchCryptoData(float &price, float &percentChange, const char* cryptoId) {
  HTTPClient http;
  String requestURL = "https://api.coingecko.com/api/v3/simple/price?ids=" + String(cryptoId) + "&vs_currencies=usd&include_24hr_change=true";
  http.begin(requestURL);
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    JsonObject obj = doc[cryptoId].as<JsonObject>();
    price = obj["usd"].as<float>();
    percentChange = obj["usd_24h_change"].as<float>();
  } else {
    Serial.println("Error on HTTP request");
  }

  http.end();
}

void displayCryptoData(float price, float percentChange, const char* cryptoName) {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 10);
  tft.printf("Currency: %s\n", cryptoName);
  tft.printf("Price: $%.2f\n", price);
  tft.setCursor(10, 60);
  tft.printf("24Hr Change: %.2f%%", percentChange);
}
