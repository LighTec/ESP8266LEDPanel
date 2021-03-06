#include <PxMatrix.h>
#include <Ticker.h>
#include <Adafruit_I2CDevice.h>

Ticker display_ticker;
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_D 12
#define P_E 0
#define P_OE 2

// Pins for LED MATRIX

//PxMATRIX display(32,16,P_LAT, P_OE,P_A,P_B,P_C);
PxMATRIX display(64,64,P_LAT, P_OE,P_A,P_B,P_C,P_D,P_E);
//PxMATRIX display(64,64,P_LAT, P_OE,P_A,P_B,P_C,P_D,P_E);

// ISR for display refresh
void display_updater()
{
  display.displayTestPattern(70);
  //display.displayTestPixel(70);
  //display.display(70);
}

uint16_t myCYAN = display.color565(0, 255, 255);

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
  display.begin(32);
  display.flushDisplay();
  display.setTextColor(myCYAN);
  display.setCursor(2,0);
  display.print("Pixel");
  Serial.println("hello");

  display_ticker.attach(0.004, display_updater);

  delay(1000);
}


void loop() {

 delay(100);

}