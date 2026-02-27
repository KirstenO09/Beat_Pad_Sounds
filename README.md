# 🎮 Beat Pad Game

## 🔧 Hardware Required
- ESP32-C3
- WS2812B LED Strip (30 LEDs)
- Jumper wires

## 🔌 Wiring
**LED Strip:**
- Data → GPIO 2  
- Power → 5V  
- Ground → GND  

## 💻 Software Setup
1. Open **Arduino IDE**
2. Install:
   - **FastLED**
   - **ESP32 Board Package**
3. Replace WiFi credentials in the code:
   ```cpp
   const char* WIFI_SSID = "YourSSID";
   const char* WIFI_PASS = "YourPassword";
