/*
Game Controller

  The circuit:
 * LCD RS pin (4) to digital pin 8
 * LCD Enable pin (6) to analog pin 0
 * LCD D4 pin (11) to analog pin 1
 * LCD D5 pin (12) to analog pin 2
 * LCD D6 pin (13) to analog pin 3
 * LCD D7 pin (14) to analog pin 4
 * LCD R/W pin (5) to GND
 * LCD VSS pin (1) to GND
 * LCD VDD pin (2) to 5V
 * LCD Red pin (16) to digital pin 3
 * LCD Green pin (17) to digital pin 5
 * LCD Blue pin (18) to digital pin 6
 * 
 * 10K potentiometer:
 *  ends to +5V and ground
 *  wiper to LCD VO pin (3)

 * 
 * Left button to digital pin 4
 * Middle button to digital pin 7
 * Right button to analog pin 5
 * All buttons N.O. to GND (black)
 * 
 * Neopixel data (blue) to digital pin 9
 * Neopixel Vcc (brown) to 5V
 * Neopixel GND (black) to GND

 */

/*  WHAT IT DOES
 * use L/R buttons to scroll through puzzles
 * use middle button to select
 * 
 * THEN, use L/R buttons to scroll through options
 * use middle button to select
 * 
 * THEN, use L/R buttons to say NO or YES when prompted
 */

// include libraries:
#include <LiquidCrystal.h>
#include <FastLED.h>

// initialize the lcd with the numbers of the interface pins
LiquidCrystal lcd(8, A0, A1, A2, A3, A4);

// for LCD backlight color
const uint8_t redPin = 3;
const uint8_t greenPin = 5;
const uint8_t bluePin = 6;

// FastLED etiquette
#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

// Initialize neopixels
#define DATA_PIN    9
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    6

CRGB leds[NUM_LEDS];

#define BRIGHTNESS 96
#define FRAMES_PER_SECOND 120

// pushbutton logic
const uint8_t buttonPins[] = {4,7,A5};
bool buttonState[] = {HIGH, HIGH, HIGH};
bool lastButtonState[] = {HIGH, HIGH, HIGH};
bool risingEdge[] = {LOW, LOW, LOW};
bool buttonPress = 0;

const uint8_t numButtons = 3;
const int left = 0;
const int middle = 1;
const int right = 2;

/*These are the commands I'm envisioning:
 * "enable" = enable
 * "solve" = enable + solve
 * "skip" = once it is enabled, insta-solve 
 * "stop" = pretend something ran to completion
 * "play" = force it to play as though it didn't play yet
 * "unlock" = disengage mag lock
 * "lock" = re-engage mag lock
 * "turn on" = allow thing to be powered
 * "turn off" = stop powering the thing
 * "RESET" = just for debugging purposes
 */ 
 
const char *puzzleArray[17][5]{
  {"Intro Seq","stop","play","(back)","(back)"},
  {"Lineage","enable","solve","skip","(back)"},
  {"Village Map","enable","solve","skip","(back)"},
  {"Cons Door","unlock","lock","(back)","(back)"},
  {"Clock","enable","solve","skip","(back)"},
  {"Pathway","enable","solve","skip","(back)"},
  {"SGK Poem","skip","(back)","(back)","(back)"},
  {"SGK Clock","skip","(back)","(back)","(back)"},
  {"SGK Chimes","skip","(back)","(back)","(back)"},
  {"Pump","enable","solve","skip","(back)"},
  {"SR Door","unlock","lock","(back)","(back)"},
  {"Heal Spell","turn on","turn off","(back)","(back)"},
  {"Moon bounce","turn on","turn off","(back)","(back)"},
  {"Mariphasa","unlock","lock","(back)","(back)"},
  {"Nightshade","unlock","lock","(back)","(back)"},
  {"Cauldron","enable","solve","skip","(back)"},
  {"Finale","stop","play","RESET","(back)"},
};

const char *gameState[18]={ //for info coming from master
"PRELUDE",
"WOLF_INTRO",
"LINEAGE",
"COIN",
"VILLAGE",
"LANTERNS",
"PATH",
"KEYBOARD_POEM",
"KEYBOARD_CLOCK",
"KEYBOARD_CHIMES",
"PIPE_WAIT",
"BOOK",
"MOON",
"CAULDRON",
"SAFEROOM",
"CURE",
"KILL",
"ESCAPE"
};

uint32_t debugColors[]={0x111111,0x7F0000,0x7F7F00,0x007F00,0x00007F};
// red,yellow,green,blue = 1st, 2nd, 3rd, 4th options

uint8_t rows = 17;
uint8_t cols = 5;
uint8_t masterState = 0; //for interpreting info from master

bool selecting = 1;
bool executing = 0;
bool confirming = 0;

uint8_t selector = 0;
uint8_t executor = 0;

void setup() {
  delay(500);

  //neopixels
  FastLED.setMaxPowerInVoltsAndMilliamps(5,500);
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  for(int i=0; i<NUM_LEDS; i++){
    leds[i] = 0x111111;
  }
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");

  //Backlight RGB
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  
  setBacklightRGB(255,255,255);

  //Initialize pushbuttons
  for(uint8_t i = 0; i<numButtons; i++){
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  screenUpdater();
}

void loop() {
  FastLED.show(); // update the LEDs

  readButtons();

  if(selecting) {
    setBacklightRGB(255,255,255);
    
    if(risingEdge[middle]){      
      executor = 1;
      setExecuting();
    }
    
    else{
      if(risingEdge[left] && selector > 0){
          selector--;        
      }
      if(risingEdge[right] && selector < (rows-1)){
          selector++;
      }
    }
  }

  else if(executing){
    setBacklightRGB(0,255,0);
    
    if(risingEdge[middle]){
      
      if(puzzleArray[selector][executor]=="(back)"){
        setSelecting();
        executor = 0;
      }
      else{
        setConfirming();
      }
      
    }
    
    else{
      
      if(risingEdge[left] && executor > 1){
          executor--;        
      }
      if(risingEdge[right] && executor < (cols-1) && puzzleArray[selector][executor]!="(back)"){
          executor++;        
      }
      
    }
  }

  else if(confirming){
    setBacklightRGB(255,0,0);
    
    if(risingEdge[middle] || risingEdge[left]){
      setExecuting();
    }
    
    if(risingEdge[right]){
      
      if(puzzleArray[selector][executor]=="RESET"){
        reset();
      }
      else{
        neopixelDebug();
        setSelecting();
        executor = 0;
      }
      
    }
  }

  if(buttonPress){
    screenUpdater();
    resetRisingEdges();
  }
  
  updateLastButtonStates();
  
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void setBacklightRGB(uint8_t redVal, uint8_t greenVal, uint8_t blueVal){
  analogWrite(redPin,map(redVal, 0, 255, 255, 0));
  analogWrite(greenPin,map(greenVal, 0, 255, 255, 0));
  analogWrite(bluePin,map(blueVal, 0, 255, 255, 0));
}

void updateLastButtonStates(){
  for(uint8_t i = 0; i<numButtons; i++){
    lastButtonState[i] = buttonState[i];
  }
}

void resetRisingEdges(){
    for(uint8_t i = 0; i<numButtons; i++){
      risingEdge[i] = LOW;
    }
}

void setSelecting(){
  selecting = 1;
  executing = 0;
  confirming = 0;
}

void setExecuting(){
  selecting = 0;
  executing = 1;
  confirming = 0;
}

void setConfirming(){
  selecting = 0;
  executing = 0;
  confirming = 1;
}

void screenUpdater(){
  lcd.clear();
  lcd.setCursor(0,0);

  if(selecting){
    lcd.print(gameState[masterState]);
    lcd.setCursor(0,1);
    lcd.print("< ");
    lcd.print(puzzleArray[selector][0]);
    lcd.print(" >");
  }
  
  else if(executing){
    lcd.print(puzzleArray[selector][0]);
    lcd.print("...");
    lcd.setCursor(0,1);
    lcd.print("< ");
    if(executor == 0){
      lcd.print("Select");
    }
    else{
      lcd.print(puzzleArray[selector][executor]);
    }
    lcd.print("?");
    lcd.print(" >");
  }
  
  else if(confirming){
   lcd.print("ARE YOU SURE?"); 
   lcd.setCursor(0,1);
   lcd.print(" NO    NO   YES ");
  }
  
}

void readButtons(){
  buttonPress = 0;
  for(uint8_t i = 0; i<numButtons; i++){
    buttonState[i] = !digitalRead(buttonPins[i]);
    if(buttonState[i] != lastButtonState[i] && buttonState[i]){
      risingEdge[i] = HIGH;
      buttonPress = 1;
    }
  }
}

void neopixelDebug(){
  //number of neopixels lit up = binary code for which puzzle got executed
  //color = which command was selected RYGB = 1/2/3/4
  for(int i = 0; i<NUM_LEDS; i++){
    leds[i]=bitRead((selector+1),i)*debugColors[executor];
  }
}

void reset(){
  selecting = 1;
  executing = 0;
  confirming = 0;

  selector = 0;
  executor = 0;
  buttonPress = 0;

  for(int i = 0; i<numButtons; i++){
    buttonState[i] = HIGH;
    lastButtonState[i] = HIGH;
    risingEdge[i] = LOW;
  }
  
  for(int i=0; i<NUM_LEDS; i++){
    leds[i] = 0x111111;
  }
}
