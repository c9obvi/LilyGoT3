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

## Fixing Library Import Issue for QR Code Generation

> [!IMPORTANT]  
> Crucial information regarding import issue for QR Code generation -- necessary for users to succeed.

When integrating the QR code generation library by Richard Moore into the ESP32 project, a naming conflict issue was encountered, preventing the library from being correctly recognized and used. The issue was resolved by renaming the library files and adjusting the project to reference the new names.

### Steps to Resolve the Naming Conflict:

1. **Rename Library Files**: The original `QRCode.h` and `QRCode.c` files were renamed to `qrcoderm.h` and `qrcoderm.c`, respectively. This step avoids naming conflicts with other components or libraries.

2. **Update Include Directives**: In the project, the include directive was changed to match the new filename:
   ```cpp
   #include <qrcoderm.h>
   ```

3. **Adjust the Library's Folder Name**: The library folder was renamed to `qrcoderm` to match the new naming scheme and ensure consistency.

4. **Modify Include in C File**: In the `qrcoderm.c` file, the include statement was updated to reflect the new header file name:
   ```c
   #include "qrcoderm.h"
   ```

5. **Compilation and Testing**: After these adjustments, the project compiled successfully, and the import issue was resolved.

### Implementation Notes:

This solution was specifically tailored to resolve a naming conflict encountered during the project's development. It highlights the importance of carefully managing library names and include directives, especially when integrating third-party libraries into complex projects.

Should similar issues arise, consider following these steps to diagnose and resolve naming conflicts, ensuring smooth library integration and functionality.


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
