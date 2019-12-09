
// Secret Santa Machine version 2.0 December 2019 by Sam Lanyon. Please reuse the code, provided you 
// acknowledge the originators license requiresemtns for the display and printer code included.
// Thank you to Jowan for the sorting algorithm.

//************** Sorting algorithm ***********************
// Put the list of people participating in Secret Santa in this string.
String people[] =
{
"Darth Vader",
"The Queen",
"Dolly Parton",
"Will Smith",
"Frodo Baggins"
};


static int numb_people = (int)sizeof(people)/sizeof(people[0]);
long randNumber;
int people_used[100];
//**************************************************************************


/*********** PRINTER ****************
 Based on
 Spark Fun Electronics 2011
 Nathan Seidle
 
This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
To use this example code, attach
 Arduino : Printer
 D6 : Green wire (Printer TX)
 D7 : Yello wire (Printer RX)
 VIN (9V wall adapter) : Red wire
 GND : Black wires
 Open Arduino serial window at 38400bps. Turn on line endings!
 
*/

#include <SoftwareSerial.h>
SoftwareSerial Thermal(6, 7); //Soft RX from printer on D2, soft TX out to printer on D3

#define FALSE  0
#define TRUE  1
int printOnBlack = FALSE;
int printUpSideDown = FALSE;

int ledPin = 4;
int heatTime = 255; //80 is default from page 23 of datasheet. Controls speed of printing and darkness
int heatInterval = 255; //2 is default from page 23 of datasheet. Controls speed of printing and darkness
char printDensity = 15; //Not sure what the defaut is. Testing shows the max helps darken text. From page 23.
char printBreakTime = 15; //Not sure what the defaut is. Testing shows the max helps darken text. From page 23.


/*********************** DISPLAY ***************************************

Based on code example code for Works with the Adafruit 1.8" TFT Breakout w/SD card
    ----> http://www.adafruit.com/products/358
    Check out the links above for our tutorials and wiring diagrams.
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional).

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
*/

#include <Adafruit_GFX.h>    // DISPLAY Core graphics library
#include <Adafruit_ST7735.h> // DISPLAY Hardware-specific library for ST7735
#include <SPI.h>             // DISPLAY
#include <Fonts/FreeSansBold9pt7b.h>  // DISPLAY

// DISPLAY 1.8" TFT.
  #define TFT_CS        10
  #define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
  #define TFT_DC         8

// OPTION 1 (recommended) is to use the HARDWARE SPI pins, which are unique
// to each board and not reassignable. For Arduino Uno: MOSI = pin 11 and
// SCLK = pin 13. This is the fastest mode of operation and is required if
// using the breakout board's microSD card.

// For 1.8" TFT with ST7735 use:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// OPTION 2 lets you interface the display using ANY TWO or THREE PINS,
// tradeoff being that performance is not as fast as hardware SPI above.
//#define TFT_MOSI 11  // Data out
//#define TFT_SCLK 13  // Clock out
//**************************************************************************/



void setup() {        
 
  pinMode(4,INPUT_PULLUP);    // The giant red button is in digital pin 4.
                             // We are enabling the built in 20k pull up resistor
                             // and the momentary button will pull the pin down to
                             // ground when its pressed.

  Serial.begin(9600);
  Serial.println(F("Hello! ST77xx TFT Test"));
  Serial.print("Numb people: ");
  Serial.println(numb_people);
  // Super important or will not get random.
  randomSeed(analogRead(0));  // uses random noise on input to Analogue input pin.
  
//********************* PRINTER *********
  
  pinMode(ledPin, OUTPUT);
  Thermal.begin(19200); //Setup soft serial for ThermalPrinter control
  printOnBlack = FALSE;
  printUpSideDown = FALSE;

  //Modify the print speed and heat
  Thermal.write(27);
  Thermal.write(55);
  Thermal.write(7); //Default 64 dots = 8*('7'+1)
  Thermal.write(heatTime); //Default 80 or 800us
  Thermal.write(heatInterval); //Default 2 or 20us

  //Modify the print density and timeout
  Thermal.write(18);
  Thermal.write(35);
  int printSetting = (printDensity<<4) | printBreakTime;
  Thermal.write(printSetting); //Combination of printDensity and printBreakTime

  Serial.println();
  Serial.println("Printer Parameters set");

//  Thermal.print("Secret Santa with ");
//  Thermal.println(numb_people);
  Thermal.write(10); //Sends the LF to the printer, advances the paper
//*************************************
  
  //********** DISPLAY STUFF ************
  // Use this initializer if using a 1.8" TFT screen:
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab
  tft.setRotation(1);
  tft.cp437(1); //Enable (or disable) Code Page 437-compatible charset. Disable to use with old code for this display.
  tft.setFont(&FreeSansBold9pt7b);  // set font
  Serial.println(F("Display initialized"));
  
 
  
  tft.fillScreen(ST77XX_BLACK);
  
  // large block of text
  tft.fillScreen(ST77XX_BLACK);

  tft.setCursor(33,30);
  testdrawtext("Welcome to", ST77XX_WHITE);
  
  tft.setCursor(30,55);
  //tft.setTextSize(2);
  testdrawtext("Secret Santa", ST77XX_RED);

  tft.setCursor(30,110);
  //tft.setTextSize(2);
  testdrawtext("Press Button", ST77XX_BLUE);
  
   

  
 // tft.invertDisplay(true);
 // delay(500);
//  tft.invertDisplay(false);
 // delay(500);
  //*************************************  

  delay(1000);
        
        while (digitalRead(4) == HIGH){       // trap here untill the button is pressed.
    
        }

  assign_santas();
  

}

void loop() {

 }



void shuffleArray(String * array, int size){    // array shuffle used in assign_santas function.
  int last = 0;
  String temp = array[last];
  for (int i=0; i<size; i++) {
    int index = random(size);
    array[last] = array[index];
    last = index;
  }
  array[last] = temp;
}


void assign_santas() {                          // assigning people function
  
  // Shuffle the people.
  shuffleArray(people, numb_people);
 
  // Loop though and assign to right, except the last person.
  for (int x = 0; x < (numb_people-1); x++){
    Serial.print(people[x]);
    Serial.print(" : ");
    Serial.println(people[x+1]);

    displayNameonScreen(x);
    delay(1000);
    waitforButton();
    printTicket(x,x+1);
    delay(4000); 
  }
  // Assign last person to the first person.
  Serial.print(people[numb_people-1]); 
  Serial.print(" : ");
  Serial.println(people[0]);
  Serial.println(" "); 

  displayNameonScreen(numb_people-1);
  delay(1000);
  waitforButton();
  printTicket(numb_people-1,0);
  delay(4000);
    

  tft.fillScreen(ST77XX_BLACK);   // make whole screen black
  tft.setCursor(10,50);
  testdrawtext("All done thanks", ST77XX_WHITE);
}


void printTicket(int c, int d){
        Thermal.write(27);    // these three lines issuing command ESC ! n from printer manual and setting double height ansd doube width for text.
        Thermal.write(33);
        Thermal.write(48);
        
        Thermal.println(people[c]);
        Thermal.write(10); //Sends the LF to the printer, advances the paper
        Thermal.print("* * * Your * * *");
        Thermal.write(10); //Sends the LF to the printer, advances the paper
        Thermal.print("Secret Santa is:");
        Thermal.write(10); //Sends the LF to the printer, advances the paper
        Thermal.write(10); //Sends the LF to the printer, advances the paper
        Thermal.println(people[d]);
        Thermal.write(10); //Sends the LF to the printer, advances the paper
        Thermal.write(10); //Sends the LF to the printer, advances the paper
        Thermal.print("- - - - - - - -");
        Thermal.write(10); //Sends the LF to the printer, advances the paper
        Thermal.write(10); //Sends the LF to the printer, advances the paper

        tft.fillScreen(ST77XX_BLACK);   // make whole screen black
        tft.setCursor(10,50);
        testdrawtext("Printing...", ST77XX_WHITE);  
}


void waitforButton(){
  while (digitalRead(4) == HIGH){       // trap here untill the button is pressed.
  }
}


void displayNameonScreen(int a){
    // large block of text
    tft.fillScreen(ST77XX_BLACK);   // make whole screen black
    tft.setCursor(20,30);
        
    tft.setTextColor(ST77XX_RED);
    tft.setTextWrap(true);
    tft.print(people[a]);
        
    tft.setCursor(10,70);
    testdrawtext("Press the button", ST77XX_WHITE);
    tft.setCursor(15,90);
    testdrawtext("for your ticket", ST77XX_WHITE);

}




void testdrawtext(char *text, uint16_t color) {   //display function
 // tft.setCursor(0,30);
 // tft.setTextSize(2);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void tftPrintTest() {                           //display function
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.println("Hello World!");
  tft.setTextColor(ST77XX_BLUE);
  tft.setTextSize(4);
  tft.print(1234.567);
  delay(1500);
  tft.setCursor(0, 0);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(0);
  tft.println("Hello World!");
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
 // tft.print(p, 6);
  tft.println(" Want pi?");
  tft.println(" ");
  tft.print(8675309, HEX); // print 8,675,309 out in HEX!
  tft.println(" Print HEX!");
  tft.println(" ");
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Sketch has been");
  tft.println("running for: ");
  tft.setTextColor(ST77XX_MAGENTA);
  tft.print(millis() / 1000);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(" seconds.");
}
