# First version 

The TTGO T-Display ESP32 ( https://www.amazon.com/gp/product/B07XQ5G279/ref=ppx_yo_dt_b_asin_title_o02_s01?ie=UTF8&psc=1 ) is a nice ESP32 board with a 240x135 color display. I couldn't find any ESP32 IDF driver code to work with it so... 

##How to get started: 
1) Get ESP-IDF. 

https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#step-2-get-esp-idf 
See if you can run a basic example

2) clone

git clone https://github.com/rdorbolo/esp32-ttgo-display.git

3) build

cd esp32-ttgo-display

idf.py build

4) run

idf.py -p (PORT) flash

Based on the project https://github.com/Xinyuan-LilyGO/TTGO-T-Display. I removed the all the arduino, #ifdefs, references to SPI, and cleaned up the initialization. 
