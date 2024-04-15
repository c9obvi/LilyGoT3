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
const int confirmButtonPin = 0; // Normally the boot button, used to confirm user choices

unsigned long previousMillis = 0; // Stores the last time the update was made
const long interval = 60000; // Interval at which to refresh (milliseconds, 60 seconds)

float price = 0; // Declare price globally
float percentChange = 0; // Declare percentChange globally

void setup() {
    Serial.begin(115200);
    pinMode(buttonPin, INPUT_PULLUP);
    
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(1);

    // Check if the main button is pressed on startup
    bool resetPressed = digitalRead(buttonPin) == LOW;
    delay(100); // Debounce delay
    resetPressed &= digitalRead(buttonPin) == LOW; // Check again to confirm

    if (resetPressed) {
        displayResetConfirmationScreen(); // Function that handles reset confirmation
    }

    WiFiManager wifiManager;
    if (!wifiManager.autoConnect("CryptoWatcherAP")) {
        Serial.println("Failed to connect and hit timeout");
        tft.fillScreen(TFT_BLACK);
        displayQRCodeForSSID("CryptoWatcherAP"); 
    } else {
        Serial.println("Connected to Wi-Fi");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        tft.drawString("Crypto Watcher", 75, 40, 4);
        tft.drawString("By 0xBerto", 95, 70, 4);
        tft.drawString("Connected to WiFi", 65, 110, 4);
        delay(3000);
    }

    fetchCryptoData(price, percentChange, cryptoIds[currentCryptoIndex]);
    displayCryptoData(price, percentChange, cryptoNames[currentCryptoIndex]);
}

void loop() {
    unsigned long currentMillis = millis();
    if (digitalRead(buttonPin) == LOW && currentMillis - previousMillis > 200) {
        previousMillis = currentMillis;
        currentCryptoIndex = (currentCryptoIndex + 1) % numCryptos;
        fetchCryptoData(price, percentChange, cryptoIds[currentCryptoIndex]);
        displayCryptoData(price, percentChange, cryptoNames[currentCryptoIndex]);
    }
}

void setBrightness(int brightness) {
    ledcWrite(0, brightness);
    Serial.print("Brightness set to: ");
    Serial.println(brightness);
}

void displayResetConfirmationScreen() {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.println("Reset WiFi Settings?");
    tft.println("CONTINUE pressing MAIN button");
    tft.println("OR");
    tft.println("RELEASE to cancel");

    // Wait for confirmation or cancellation
    delay(5000); // Give time for decision, adjust as necessary

    if (digitalRead(buttonPin) == LOW) { // Check if still pressed
        clearWiFiCredentials();
    } else {
        tft.fillScreen(TFT_BLACK);
        tft.println("Reset Canceled");
        delay(2000);
    }
}

void clearWiFiCredentials() {
    WiFi.disconnect(true); // Clear credentials
    Serial.println("WiFi credentials cleared.");
    tft.fillScreen(TFT_BLACK);
    tft.println("Credentials Cleared");
    delay(2000);
    ESP.restart(); // Restart the device to apply changes
}

void displayQRCodeForSSID(const char* ssid) {
    String qrCodeContent = "WIFI:T:nopass;S:" + String(ssid) + ";;";
    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(3)];
    qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, qrCodeContent.c_str());
    int scale = 4;
    int qrSize = qrcode.size * scale;
    int startX = (tft.width() - qrSize) / 2;
    int startY = (tft.height() - qrSize) / 2;
    tft.fillScreen(TFT_BLACK);
    for (int y = 0; y < qrcode.size; y++) {
        for (int x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                tft.fillRect(startX + x * scale, startY + y * scale, scale, scale, TFT_WHITE);
            }
        }
    }
    tft.drawString("Scan to connect", startX - 30, startY + qrSize + 10, 2);
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
    tft.drawString("Currency: " + String(cryptoName), 10, 10, 2);
    tft.drawString("Price: $" + String(price, 2), 10, 50, 2);
    tft.drawString("24Hr Change: " + String(percentChange, 2) + "%", 10, 90, 2);
}
