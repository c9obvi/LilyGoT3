#include <WiFiManager.h> 
#include <TFT_eSPI.h>     
#include <ArduinoJson.h>  
#include <HTTPClient.h>
#include <qrcoderm.h>

// Define color constants
#define TFT_BLACK 0x0000
#define TFT_WHITE 0xFFFF
#define TFT_RED   0xF800
#define TFT_GREEN 0x07E0

// Instantiate display and sprite
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);  // Create top right sprite object for dynamic content
TFT_eSprite bottomSprite = TFT_eSprite(&tft);  // Create another sprite object for the bottom left content

// Button and brightness control
const int buttonPin = 14;
const int confirmButtonPin = 0;
int brightnessLevels[] = {64, 128, 192, 255};
int brightnessIndex = 0;
bool brightnessChanged = false;  // Flag to track brightness changes
int currentSetBrightness = 0;  // This will store the last brightness level set via PWM.

// Crypto data variables
const char* cryptoIds[] = {"bitcoin", "ethereum", "dogecoin", "monero"};
const char* cryptoNames[] = {"BTC", "ETH", "DOGE", "XMR"};
int currentCryptoIndex = 0;
float price = 0;
float percentChange = 0;

void setup() {
    Serial.begin(115200);
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(confirmButtonPin, INPUT_PULLUP);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(1);

    // Brightness setup using PWM
    ledcSetup(0, 5000, 8);  // 5kHz PWM, 8-bit resolution might be more stable
    ledcAttachPin(38, 0);  // Ensure pin 38 is correct for your board and setup
    setBrightness(brightnessLevels[brightnessIndex]);

    // Initialize top right sprite for dynamic content
    sprite.createSprite(80, 64); // Size of the sprite
    sprite.setPivot(320, 0);  // Top right corner of the screen

    // Initialize bottom left sprite for static or less frequent updates
    bottomSprite.createSprite(237, 64); // 2.5 times the width of the top sprite
    bottomSprite.setPivot(0, tft.height() - 64);  // Bottom left corner of the screen
    

    // displayBottomSpritePlace();  // Draw initial content for the bottom sprite

    WiFiManager wifiManager;
    if (!wifiManager.autoConnect("CryptoWatcherAP")) {
        Serial.println("Failed to connect and hit timeout");
        displayQRCodeForSSID("CryptoWatcherAP");
    } else {
        showSplashScreen();
        delay(1500); // Display splash screen for 1.5 seconds
    }

    fetchCryptoData(price, percentChange, cryptoIds[currentCryptoIndex]);
    displayCryptoData(price, percentChange, cryptoNames[currentCryptoIndex]);
}

void loop() {
    handleButtonInputs();
    if (brightnessChanged) {
        updateLightBar();
        brightnessChanged = false;  // Reset flag after updating
    }

    // Periodic brightness correction
    static unsigned long lastBrightnessCheckMillis = 0;
    const unsigned long brightnessCheckInterval = 1000;  // Check every second
    if (millis() - lastBrightnessCheckMillis > brightnessCheckInterval) {
        ledcWrite(0, currentSetBrightness);  // Re-apply the current brightness setting to correct any drift
        lastBrightnessCheckMillis = millis();
    }
}

// user input logic
void handleButtonInputs() {
    static unsigned long lastButtonPress = 0;
    const unsigned long debounceTime = 200;  // Debounce time in milliseconds

    if (millis() - lastButtonPress < debounceTime) {
        return;  // Exit if the debounce period has not passed
    }

    if (digitalRead(buttonPin) == LOW) {
        lastButtonPress = millis();
        currentCryptoIndex = (currentCryptoIndex + 1) % (sizeof(cryptoIds) / sizeof(cryptoIds[0]));
        fetchCryptoData(price, percentChange, cryptoIds[currentCryptoIndex]);
        displayCryptoData(price, percentChange, cryptoNames[currentCryptoIndex]);
    }

    if (digitalRead(confirmButtonPin) == LOW) {
        lastButtonPress = millis();
        brightnessIndex++;
        if (brightnessIndex >= (sizeof(brightnessLevels) / sizeof(brightnessLevels[0]))) {
            brightnessIndex = 0;
        }
        setBrightness(brightnessLevels[brightnessIndex]);
        brightnessChanged = true;  // Set flag to true as brightness has changed
    }
}

// Brightness function
void setBrightness(int brightness) {
    ledcWrite(0, brightness);
    currentSetBrightness = brightness;  // Update the last set brightness level.
    Serial.print("Brightness set to: ");
    Serial.println(brightness);
    brightnessChanged = true;  // Ensure the light bar updates.
}
void updateLightBar() {
    // Adjusted dimensions for a smaller light bar
    int barX = 314, barY = 80, barWidth = 4, barHeight = 80; // Reduced height
    int segmentHeight = 18, segmentSpacing = 2;  // Smaller segment height

    // Clear the bar area to prepare for new drawing
    tft.drawRect(barX - 2, barY, barWidth + 4, barHeight + 4, TFT_WHITE);  // Smaller boundary for the light bar
    tft.fillRect(barX, barY + 3, barWidth, barHeight, TFT_BLACK);  // Correct method for filling a rectangle

    // Calculate the number of segments to light up
    int totalLevels = sizeof(brightnessLevels) / sizeof(brightnessLevels[0]);
    int seg = (brightnessIndex + 1) * (totalLevels / totalLevels);  // Ensure full range is utilized

    // Calculate the starting position for the first segment at the bottom of the bar
    int startY = barY + barHeight - segmentHeight; 

    // Draw each segment
    for (int i = 0; i < seg; i++) {
        tft.fillRect(barX, startY - i * (segmentHeight + segmentSpacing), barWidth, segmentHeight, TFT_GREEN);
    }
    // Add vertical text "BRIGHTNESS" top-down
    const char* text = "BRIGHTNESS";
    int letterHeight = 9;  // Reduced height of each letter for smaller text
    int textX = barX - 9;  // X position to the right of the bar, adjust as needed
    int textY = barY;  // Start at the top of the bar

    for (int i = 0; text[i] != '\0'; i++) {
    tft.drawChar(textX, textY + i * letterHeight, text[i], TFT_WHITE, TFT_BLACK, 1);  // Using smaller font size
  }
} 

// Splash Screen Nerd
void showSplashScreen() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    Serial.println("Connected to Wi-Fi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    tft.drawString("Crypto Watcher", 75, 40, 4);
    tft.drawString("By 0xBerto", 95, 70, 4);
    tft.drawString("Connected to WiFi", 65, 110, 4);
}

// QR Code logic an display for connecting to device's AP and captive portal
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

// Wifi Credential Clearing Display & Logic
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

// Fetch & Display Data Area
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
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Currency: " + String(cryptoName), 10, 10, 2);
    tft.setTextColor(percentChange >= 0 ? TFT_GREEN : TFT_RED, TFT_BLACK);
    tft.drawString("Price: $" + String(price, 2), 10, 45, 2);
    tft.drawString("Change: " + String(percentChange, 2) + "%", 10, 75, 2);

// Add a border to the top-right sprite
    sprite.drawRect(0, 0, 80, 64, TFT_WHITE); // Draw a white border around the sprite
    sprite.setTextColor(TFT_WHITE, TFT_BLACK);
    sprite.drawString("Gif", 25, 25, 1);
    sprite.drawString("Place Holder", 15, 35, 1);
    // Assuming your display is 320 pixels wide, to position the sprite at the top right:
    // The width of the sprite is 80, so we position its left edge at 320 - 80 = 240
    sprite.pushSprite(240, 0);  // Position the sprite at the top right corner
    
    // Bottom Sprite Bar
    bottomSprite.fillSprite(TFT_BLACK);
    // Draw a rounded rectangle around the border of the sprite
    // Syntax: drawRoundRect(x, y, w, h, radius, color);
    bottomSprite.drawRoundRect(1, 1, 235, 62, 5, TFT_WHITE); // Slightly smaller to fit inside the sprite
    bottomSprite.setTextColor(TFT_WHITE, TFT_BLACK);
    bottomSprite.drawString("Next SpaceX Launch..", 5, 25, 2);  // Placeholder text
    bottomSprite.pushSprite(0, tft.height() - 64);  // Push to the display
}

// void displayBottomSpritePlace() {
//     bottomSprite.fillSprite(TFT_BLACK);
//     bottomSprite.setTextColor(TFT_WHITE, TFT_BLACK);
//     bottomSprite.drawString("Future Updates Here", 5, 5, 2);  // Placeholder text
//     bottomSprite.pushSprite(0, tft.height() - 64);  // Push to the display
// }
