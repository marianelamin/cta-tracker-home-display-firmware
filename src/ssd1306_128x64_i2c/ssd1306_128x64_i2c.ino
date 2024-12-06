#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // // Show initial display buffer contents on the screen --
  // // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
 
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Hello Chicago!");
  display.display();
  delay(100);
  
}

void loop() {
  Serial.println("in loop...");

  display.startscrollright(0x00, 0x0F);
  delay(2000);
  
  display.stopscroll();
  delay(1000);
  

  display.startscrollleft(0x00, 0x0F);
  delay(2000);
  
  display.stopscroll();
  delay(1000);
  
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  
  display.stopscroll();
  delay(1000);
  
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  
  display.stopscroll();
  delay(1000);

}
