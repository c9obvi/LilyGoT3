#include <WiFiManager.h> // Include the Wi-Fi Manager library
#include <TFT_eSPI.h>     // Graphics and font library for ST7735 driver chip
#include <ArduinoJson.h>  // Include ArduinoJson library for parsing the API response
#include <HTTPClient.h>
#include <qrcoderm.h>

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
  
  tft.drawString("Crypto Watcher", 10, 10, 4);
  tft.drawString("By 0xBerto", 12, 35, 4);
  tft.drawString("Configuring WiFi...", 10, 65, 4);
  delay(3000); // Show the splash screen for a few seconds
  displayQRCodeForSSID("CryptoWatcherAP"); // Call this with your desired SSID


  WiFiManager wifiManager;
  if (!wifiManager.startConfigPortal("CryptoWatcherAP")) {
    tft.fillScreen(TFT_BLACK);
    tft.drawString("Could not find known AP", 10, 10, 4);
    tft.drawString("Turning into AP", 10, 25, 4);
    tft.drawString("Connect to WiFi: CryptoWatcherAP", 10, 40, 4);
    // Placeholder for QR code display logic
    displayQRCodeForSSID("CryptoWatcherAP"); 
    tft.println("Please scan to configure WiFi")
  } else {
    Serial.println("Connected to Wi-Fi");
    tft.fillScreen(TFT_BLACK);
    tft.drawString("Connected to WiFi", 10, 10, 4);
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    tft.drawString("IP: " + WiFi.localIP().toString(), 10, 80, 4);
  }

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

void displayQRCodeForSSID(const char* ssid) {
    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(3)]; // Adjust the version as needed
    qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, ssid); // Initialize the QR code

    // Determine scaling based on your display size and QR code size
    int scale = max(1, min(tft.width() / qrcode.size, tft.height() / qrcode.size));

    tft.fillScreen(TFT_BLACK); // Clear the screen
    
    // Iterate over the QR code modules and draw them
    for (int y = 0; y < qrcode.size; y++) {
        for (int x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                tft.fillRect(x * scale, y * scale, scale, scale, TFT_WHITE);
            } // No else part needed, as the background is already cleared
        }
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
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.printf("Currency: %s\n", cryptoName);
  tft.printf("Price: $%.2f\n", price);
  tft.setCursor(10, 60);
  tft.printf("24Hr Change: %.2f%%", percentChange);
}
