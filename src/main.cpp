#include <Arduino.h>
#include <mydebug.h>
#include <myfunction.h>

#include <TFT_eSPI.h>       // Include the graphics library
TFT_eSPI tft = TFT_eSPI();  // Create object "tft"

setup_t user;  // The library defines the type "setup_t" as a struct
			   // Calling tft.getSetup(user) populates it with the settings

#define TFT_GREY 0x7BEF

unsigned long runTime = 0;

float sx = 0, sy = 0;
uint16_t x0 = 0, x1 = 0, yy0 = 0, yy1 = 0;

unsigned int rainbow(int value) {
	// Value is expected to be in range 0-127
	// The value is converted to a spectrum colour from 0 = blue through to red = blue

	byte red = 0;    // Red is the top 5 bits of a 16 bit colour value
	byte green = 0;  // Green is the middle 6 bits
	byte blue = 0;   // Blue is the bottom 5 bits

	byte quadrant = value / 32;

	if (quadrant == 0) {
		blue = 31;
		green = 2 * (value % 32);
		red = 0;
	}
	if (quadrant == 1) {
		blue = 31 - (value % 32);
		green = 63;
		red = 0;
	}
	if (quadrant == 2) {
		blue = 0;
		green = 63;
		red = value % 32;
	}
	if (quadrant == 3) {
		blue = 0;
		green = 63 - 2 * (value % 32);
		red = 31;
	}
	return (red << 11) + (green << 5) + blue;
}

void printProcessorName(void) {
	Serial.print("Processor    = ");
	if (user.esp == 0x8266) Serial.println("ESP8266");
	if (user.esp == 0x32) Serial.println("ESP32");
	if (user.esp == 0x32F) Serial.println("STM32");
	if (user.esp == 0x2040) Serial.println("RP2040");
	if (user.esp == 0x0000) Serial.println("Generic");
}

// Get pin name
int8_t getPinName(int8_t pin) {
	// For ESP32 and RP2040 pin labels on boards use the GPIO number
	if (user.esp == 0x32 || user.esp == 0x2040) return pin;

	if (user.esp == 0x8266) {
		// For ESP8266 the pin labels are not the same as the GPIO number
		// These are for the NodeMCU pin definitions:
		//        GPIO       Dxx
		if (pin == 16) return 0;
		if (pin == 5) return 1;
		if (pin == 4) return 2;
		if (pin == 0) return 3;
		if (pin == 2) return 4;
		if (pin == 14) return 5;
		if (pin == 12) return 6;
		if (pin == 13) return 7;
		if (pin == 15) return 8;
		if (pin == 3) return 9;
		if (pin == 1) return 10;
		if (pin == 9) return 11;
		if (pin == 10) return 12;
	}

	if (user.esp == 0x32F) return pin;

	return -1;  // Invalid pin
}

void getInfo() {
	tft.getSetup(user);  //

	Serial.print("\n[code]\n");

	Serial.print("TFT_eSPI ver = ");
	Serial.println(user.version);
	printProcessorName();
#if defined(ESP32) || defined(ESP8266)
	if (user.esp < 0x32F000 || user.esp > 0x32FFFF) {
		Serial.print("Frequency    = ");
		Serial.print(ESP.getCpuFreqMHz());
		Serial.println("MHz");
	}
#endif
#ifdef ESP8266
	Serial.print("Voltage      = ");
	Serial.print(ESP.getVcc() / 918.0);
	Serial.println("V");  // 918 empirically determined
#endif
	Serial.print("Transactions = ");
	Serial.println((user.trans == 1) ? "Yes" : "No");
	Serial.print("Interface    = ");
	Serial.println((user.serial == 1) ? "SPI" : "Parallel");
#ifdef ESP8266
	if (user.serial == 1) {
		Serial.print("SPI overlap  = ");
		Serial.println((user.overlap == 1) ? "Yes\n" : "No\n");
	}
#endif
	if (user.tft_driver != 0xE9D)  // For ePaper displays the size is defined in the sketch
	{
		Serial.print("Display driver = ");
		Serial.println(user.tft_driver, HEX);  // Hexadecimal code
		Serial.print("Display width  = ");
		Serial.println(user.tft_width);  // Rotation 0 width and height
		Serial.print("Display height = ");
		Serial.println(user.tft_height);
		Serial.println();
	} else if (user.tft_driver == 0xE9D)
		Serial.println("Display driver = ePaper\n");

	if (user.r0_x_offset != 0) {
		Serial.print("R0 x offset = ");
		Serial.println(user.r0_x_offset);
	}  // Offsets, not all used yet
	if (user.r0_y_offset != 0) {
		Serial.print("R0 y offset = ");
		Serial.println(user.r0_y_offset);
	}
	if (user.r1_x_offset != 0) {
		Serial.print("R1 x offset = ");
		Serial.println(user.r1_x_offset);
	}
	if (user.r1_y_offset != 0) {
		Serial.print("R1 y offset = ");
		Serial.println(user.r1_y_offset);
	}
	if (user.r2_x_offset != 0) {
		Serial.print("R2 x offset = ");
		Serial.println(user.r2_x_offset);
	}
	if (user.r2_y_offset != 0) {
		Serial.print("R2 y offset = ");
		Serial.println(user.r2_y_offset);
	}
	if (user.r3_x_offset != 0) {
		Serial.print("R3 x offset = ");
		Serial.println(user.r3_x_offset);
	}
	if (user.r3_y_offset != 0) {
		Serial.print("R3 y offset = ");
		Serial.println(user.r3_y_offset);
	}

	if (user.pin_tft_mosi != -1) {
		Serial.print("MOSI    = ");
		Serial.print("GPIO ");
		Serial.println(getPinName(user.pin_tft_mosi));
	}
	if (user.pin_tft_miso != -1) {
		Serial.print("MISO    = ");
		Serial.print("GPIO ");
		Serial.println(getPinName(user.pin_tft_miso));
	}
	if (user.pin_tft_clk != -1) {
		Serial.print("SCK     = ");
		Serial.print("GPIO ");
		Serial.println(getPinName(user.pin_tft_clk));
	}

#ifdef ESP8266
	if (user.overlap == true) {
		Serial.println("Overlap selected, following pins MUST be used:");

		Serial.println("MOSI     = SD1 (GPIO 8)");
		Serial.println("MISO     = SD0 (GPIO 7)");
		Serial.println("SCK      = CLK (GPIO 6)");
		Serial.println("TFT_CS   = D3  (GPIO 0)\n");

		Serial.println("TFT_DC and TFT_RST pins can be user defined");
	}
#endif
	String pinNameRef = "GPIO ";
#ifdef ESP8266
	pinNameRef = "PIN_D";
#endif
	if (user.esp == 0x32F) {
		Serial.println("\n>>>>> Note: STM32 pin references above D15 may not reflect board markings <<<<<");
		pinNameRef = "D";
	}
	if (user.pin_tft_cs != -1) {
		Serial.print("TFT_CS   = " + pinNameRef);
		Serial.println(getPinName(user.pin_tft_cs));
	}
	if (user.pin_tft_dc != -1) {
		Serial.print("TFT_DC   = " + pinNameRef);
		Serial.println(getPinName(user.pin_tft_dc));
	}
	if (user.pin_tft_rst != -1) {
		Serial.print("TFT_RST  = " + pinNameRef);
		Serial.println(getPinName(user.pin_tft_rst));
	}

	if (user.pin_tch_cs != -1) {
		Serial.print("TOUCH_CS = " + pinNameRef);
		Serial.println(getPinName(user.pin_tch_cs));
	}

	if (user.pin_tft_wr != -1) {
		Serial.print("TFT_WR   = " + pinNameRef);
		Serial.println(getPinName(user.pin_tft_wr));
	}
	if (user.pin_tft_rd != -1) {
		Serial.print("TFT_RD   = " + pinNameRef);
		Serial.println(getPinName(user.pin_tft_rd));
	}

	if (user.pin_tft_d0 != -1) {
		Serial.print("\nTFT_D0   = " + pinNameRef);
		Serial.println(getPinName(user.pin_tft_d0));
	}
	if (user.pin_tft_d1 != -1) {
		Serial.print("TFT_D1   = " + pinNameRef);
		Serial.println(getPinName(user.pin_tft_d1));
	}
	if (user.pin_tft_d2 != -1) {
		Serial.print("TFT_D2   = " + pinNameRef);
		Serial.println(getPinName(user.pin_tft_d2));
	}
	if (user.pin_tft_d3 != -1) {
		Serial.print("TFT_D3   = " + pinNameRef);
		Serial.println(getPinName(user.pin_tft_d3));
	}
	if (user.pin_tft_d4 != -1) {
		Serial.print("TFT_D4   = " + pinNameRef);
		Serial.println(getPinName(user.pin_tft_d4));
	}
	if (user.pin_tft_d5 != -1) {
		Serial.print("TFT_D5   = " + pinNameRef);
		Serial.println(getPinName(user.pin_tft_d5));
	}
	if (user.pin_tft_d6 != -1) {
		Serial.print("TFT_D6   = " + pinNameRef);
		Serial.println(getPinName(user.pin_tft_d6));
	}
	if (user.pin_tft_d7 != -1) {
		Serial.print("TFT_D7   = " + pinNameRef);
		Serial.println(getPinName(user.pin_tft_d7));
	}

#if defined(TFT_BL)
	Serial.print("\nTFT_BL           = " + pinNameRef);
	Serial.println(getPinName(user.pin_tft_led));
#if defined(TFT_BACKLIGHT_ON)
	Serial.print("TFT_BACKLIGHT_ON = ");
	Serial.println(user.pin_tft_led_on == HIGH ? "HIGH" : "LOW");
#endif
#endif

	Serial.println();

	uint16_t fonts = tft.fontsLoaded();
	if (fonts & (1 << 1)) Serial.print("Font GLCD   loaded\n");
	if (fonts & (1 << 2)) Serial.print("Font 2      loaded\n");
	if (fonts & (1 << 4)) Serial.print("Font 4      loaded\n");
	if (fonts & (1 << 6)) Serial.print("Font 6      loaded\n");
	if (fonts & (1 << 7)) Serial.print("Font 7      loaded\n");
	if (fonts & (1 << 9))
		Serial.print("Font 8N     loaded\n");
	else if (fonts & (1 << 8))
		Serial.print("Font 8      loaded\n");
	if (fonts & (1 << 15)) Serial.print("Smooth font enabled\n");
	Serial.print("\n");

	if (user.serial == 1) {
		Serial.print("Display SPI frequency = ");
		Serial.println(user.tft_spi_freq / 10.0);
	}
	if (user.pin_tch_cs != -1) {
		Serial.print("Touch SPI frequency   = ");
		Serial.println(user.tch_spi_freq / 10.0);
	}

	Serial.println("[/code]");
}

// void setup() {
// 	Serial.begin(115200);
// 	// Setup the LCD
// 	tft.init();
// 	tft.setRotation(3);
// 	getInfo();
// }

// void loop() {
// 	runTime = millis();

// 	tft.fillScreen(TFT_BLACK);
// 	tft.startWrite();
// 	for (int px = 1; px < TFT_HEIGHT; px++) {
// 		for (int py = 0; py < TFT_WIDTH; py++) {
// 			float x0 = (map(px, 0, TFT_HEIGHT, -250000 / 2, -242500 / 2)) / 100000.0;  // scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.5, 1))
// 			float yy0 = (map(py, 0, TFT_WIDTH, -75000 / 4, -61000 / 4)) / 100000.0;    // scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1, 1))
// 			float xx = 0.0;
// 			float yy = 0.0;
// 			int iteration = 0;
// 			int max_iteration = 128;
// 			while (((xx * xx + yy * yy) < 4) && (iteration < max_iteration)) {
// 				float xtemp = xx * xx - yy * yy + x0;
// 				yy = 2 * xx * yy + yy0;
// 				xx = xtemp;
// 				iteration++;
// 			}
// 			int color = rainbow((3 * iteration + 64) % 128);
// 			yield();
// 			tft.drawPixel(px, py, color);
// 		}
// 	}
// 	tft.endWrite();

// 	Serial.println(millis() - runTime);
// 	while (1) yield();
// }

void setup(void) {
	DEBUGINIT();
	DEBUGLOG("%lu(%u) SETUP : Start\n",millis(),xPortGetCoreID());
//	WiFi.mode(wifi_mode_t::WIFI_MODE_STA);
	// WiFi.persistent( true );
//	WiFi.setAutoConnect(true);
	// WiFi.setAutoReconnect(true);
	DEBUGLOG("%lu(%u) SSID:%s\n", millis(),xPortGetCoreID(), WiFi.SSID().c_str());
	DEBUGLOG("%lu(%u) PWD:%s\n", millis(),xPortGetCoreID(), WiFi.psk().c_str());
	DEBUGLOG("%lu(%u) Channel:%u\n", millis(),xPortGetCoreID(), WiFi.channel());
	DEBUGLOG("%lu(%u) BSSID:%s\n", millis(),xPortGetCoreID(), WiFi.BSSIDstr().c_str());

	WiFi.begin();

	DEBUGLOG("%lu(%u) SSID:%s\n", millis(),xPortGetCoreID(), WiFi.SSID().c_str());
	DEBUGLOG("%lu(%u) PWD:%s\n", millis(),xPortGetCoreID(), WiFi.psk().c_str());
	DEBUGLOG("%lu(%u) Channel:%u\n", millis(),xPortGetCoreID(), WiFi.channel());
	DEBUGLOG("%lu(%u) BSSID:%s\n", millis(),xPortGetCoreID(), WiFi.BSSIDstr().c_str());

	for(uint8_t t = 0 ; t<30; t++){
		delay(1000);
		DEBUGLOG("[%u]", WiFi.status());
	}
	WiFi.begin("LnD", "solean230702");
	DEBUGLOG("%lu(%u) SSID:%s\n", millis(),xPortGetCoreID(), WiFi.SSID().c_str());
	DEBUGLOG("%lu(%u) PWD:%s\n", millis(),xPortGetCoreID(), WiFi.psk().c_str());
	DEBUGLOG("%lu(%u) Channel:%u\n", millis(),xPortGetCoreID(), WiFi.channel());
	DEBUGLOG("%lu(%u) BSSID:%s\n", millis(),xPortGetCoreID(), WiFi.BSSIDstr().c_str());
	for(uint8_t t = 0 ; t<30; t++){
		delay(1000);
		DEBUGLOG("[%u]", WiFi.status());
	}
//	mySmartConfig();
//	mySmartConfig(true);
	if (WiFi.begin() != wl_status_t::WL_DISCONNECTED)
		DEBUGLOG("%lu(%u) WIFI : OK\n",millis(),xPortGetCoreID());
	else
		DEBUGLOG("%lu(%u) WIFI : NOK\n",millis(),xPortGetCoreID());



//	Serial.begin(115200);
	getInfo();
	tft.init();

	tft.fillScreen(TFT_BLACK);

	// Set "cursor" at top left corner of display (0,0) and select font 4
	tft.setCursor(0, 0, 4);

	// Set the font colour to be white with a black background
	tft.setTextColor(TFT_WHITE, TFT_BLACK);

	// We can now plot text on screen using the "print" class
	tft.println("Initialised default\n");
	tft.println("White text");

	tft.setTextColor(TFT_RED, TFT_BLACK);
	tft.println("Red text");

	tft.setTextColor(TFT_GREEN, TFT_BLACK);
	tft.println("Green text");

	tft.setTextColor(TFT_BLUE, TFT_BLACK);
	tft.println("Blue text");

	delay(5000);
}

void loop() {
	tft.invertDisplay(false);  // Where i is true or false

	tft.fillScreen(TFT_BLACK);

	tft.setCursor(0, 0, 4);

	tft.setTextColor(TFT_WHITE, TFT_BLACK);
	tft.println("Invert OFF\n");

	tft.println("White text");

	tft.setTextColor(TFT_RED, TFT_BLACK);
	tft.println("Red text");

	tft.setTextColor(TFT_GREEN, TFT_BLACK);
	tft.println("Green text");

	tft.setTextColor(TFT_BLUE, TFT_BLACK);
	tft.println("Blue text");

	delay(5000);

	// Binary inversion of colours
	tft.invertDisplay(true);  // Where i is true or false

	tft.fillScreen(TFT_BLACK);

	tft.setCursor(0, 0, 4);

	tft.setTextColor(TFT_WHITE, TFT_BLACK);
	tft.println("Invert ON\n");

	tft.println("White text");

	tft.setTextColor(TFT_RED, TFT_BLACK);
	tft.println("Red text");

	tft.setTextColor(TFT_GREEN, TFT_BLACK);
	tft.println("Green text");

	tft.setTextColor(TFT_BLUE, TFT_BLACK);
	tft.println("Blue text");

	delay(5000);
}
