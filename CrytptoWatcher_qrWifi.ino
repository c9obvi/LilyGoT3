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

  WiFiManager wifiManager;
  
  // Attempt to auto-connect to previously saved WiFi without starting the config portal
  wifiManager.setConfigPortalTimeout(180); // Timeout for trying to connect to saved WiFi; adjust as needed
  if(!wifiManager.autoConnect("CryptoWatcherAP")) {
    Serial.println("Failed to connect and hit timeout");
    // Failed to connect to WiFi and hit timeout
    // Clear the screen and display the QR code
    tft.fillScreen(TFT_BLACK);
    displayQRCodeForSSID("CryptoWatcherAP"); 
  } else {
    // Successfully connected to WiFi
    tft.drawString("Crypto Watcher", 75, 40, 4);
    tft.drawString("By 0xBerto", 95, 70, 4);
    tft.drawString("Connected to WiFi", 65, 110, 4);
    delay(3000); // Show the connected message for a few seconds
    Serial.println("Connected to Wi-Fi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
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
    // Format the string for an open WiFi network QR code
    String qrCodeContent = "WIFI:T:nopass;S:" + String(ssid) + ";;";

    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(3)]; // Adjust the version as needed
    // Use qrCodeContent.c_str() to convert the String to a C-style string (char array)
    qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, qrCodeContent.c_str()); // Initialize the QR code

    int scale = 4; // Adjust scale based on your display size and desired QR code size
    int qrSize = qrcode.size * scale; // Total QR size in pixels

    // Calculate starting position to center the QR code
    int startX = (tft.width() - qrSize) / 2;
    int startY = (tft.height() - qrSize) / 2;

    tft.fillScreen(TFT_BLACK); // Clear the screen

    // Draw each module of the QR code
    for (int y = 0; y < qrcode.size; y++) {
        for (int x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                tft.fillRect(startX + x * scale, startY + y * scale, scale, scale, TFT_WHITE);
            }
        }
    }

    // Display instruction text below the QR code
    tft.setCursor(startX - 30, startY + qrSize + 10); // Adjust y position to be just below the QR code
    tft.setTextColor(TFT_WHITE, TFT_BLACK); // Set text color; adjust as needed
    tft.setTextSize(2); // Adjust text size as needed
    tft.println("Scan to connect");
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
