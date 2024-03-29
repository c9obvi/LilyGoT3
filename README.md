# LilyGo T-Display S3 Projects

This repository is dedicated to Arduino sketches (`*.ino` files) specifically designed for the LilyGo T-Display S3, a versatile and powerful ESP32-based development board with a built-in TFT display. The main focus of this repository is to share and document projects that leverage the unique capabilities of the T-Display S3, from displaying information on the screen to interacting with various sensors and internet services.

## About the LilyGo T-Display S3

The LilyGo T-Display S3 is part of the LilyGo T-Display series, featuring an ESP32 chip with Wi-Fi and Bluetooth capabilities, along with a colorful TFT display for visual output. It is an excellent platform for IoT projects, educational purposes, and DIY electronics enthusiasts looking to create interactive and connected devices.

### Configuring the TFT_eSPI Library

For the projects in this repository to work correctly with your LilyGo T-Display S3, you'll need to configure the TFT_eSPI library to use the correct display driver. Follow these steps:

1. Navigate to your Arduino libraries directory, typically found at `~/Documents/Arduino/libraries/TFT_eSPI`.
2. Open the `User_Setup_Select.h` file for editing. You can use `nano` if you're on a Unix-like system:
   ```
   nano User_Setup_Select.h
   ```
3. Comment out the current display driver selection by adding `//` at the beginning of the line.
4. Uncomment the line corresponding to the display driver used by your LilyGo T-Display S3 by removing `//` from the beginning of the appropriate line.
5. Save the file and exit the editor. If you're using `nano`, you can do this by pressing `CTRL + X`, then `Y` to confirm saving, and `Enter` to exit.

This configuration step ensures that the TFT_eSPI library communicates correctly with the display on your LilyGo T-Display S3.

## Projects in This Repository

### Bitcoin Price Tracker

- **File**: `Bitcoin_Price_Tracker.ino`
- **Description**: This sketch connects to the internet via Wi-Fi to fetch and display the current price of Bitcoin, along with the 24-hour percentage change. It demonstrates how to perform HTTP GET requests, parse JSON data, and update the T-Display S3 screen accordingly.

### Cryptocurrency Price Cycle

- **File**: `Crypto_Price_Cycle.ino`
- **Description**: Expanding upon the Bitcoin tracker, this project allows cycling through multiple cryptocurrencies, including Bitcoin, Ethereum, Dogecoin, and Monero, by pressing a button. It showcases handling user input, dynamic data fetching, and display updates on the T-Display S3.

## Getting Started

To use the sketches in this repository, you'll need the Arduino IDE with ESP32 support installed, along with the following libraries:

- TFT_eSPI
- ArduinoJson
- WiFi

Make sure to configure the `TFT_eSPI` library for the T-Display S3 according to the instructions provided in the library's documentation.

## Contributing

Contributions to this repository are welcome! Whether it's adding new projects, improving existing ones, or reporting issues, your input helps make this resource better for everyone.

## License

The projects in this repository are released under the MIT License. See the `LICENSE` file in the repository for more details.
