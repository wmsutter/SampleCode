// A very simple example skecth that allows the  onbaord LED to be reomotely 
// controlled by a webpage. See http://wp.josh.com/2014/05/01/a-platform-for-casual-encounters/ for more info.

#include <Process.h>

// Neopixel libraries
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#define PINA1 6 // R1_blw, 1_blw (Red,Blue)
#define NUM_LEDSA1 140
#define PINA0 5 // R1_blu, 1_blu (Red,Blue)
#define NUM_LEDSA0 10

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

Adafruit_NeoPixel stripBig = Adafruit_NeoPixel(NUM_LEDSA1, PINA1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripSmall = Adafruit_NeoPixel(NUM_LEDSA0, PINA0, NEO_GRB + NEO_KHZ800);

// This is the default baud rate for the serial link 
#define LININOBAUD 250000  

// Use the onboard LED
#define LED_PIN 13

int ledState = LOW;
int usecase = 1;
int flag[] = {0,0,0,0,0,0};
int buttonPin = 3;
int magPin = 9;

int firsttimeFlicker = 1;
int firsttimeParty = 1;

int justToggled = 0;
int buttonState;
int lastReading;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 100;

Process p;

void setup() {
    stripBig.begin(); //6
    stripBig.show();
    stripSmall.begin(); //7
    stripSmall.show();
    
    Bridge.begin();
    Serial.begin(9600);
    Serial1.begin(LININOBAUD); // open serial connection to Linino
    pinMode( LED_PIN, OUTPUT );
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(magPin, OUTPUT);
    digitalWrite(magPin, HIGH);
    lastButtonState = digitalRead(buttonPin);
}

// We to use this string to indicate where the command is inside the request string...
// Mast match the command string sent by the HTML code
#define COMMAND_PREFIX "COMMAND="


// Understood command string formats:
// '0' - Turn of LED
// '1' - Turn on LED
// '2' - play audio on usb

// This is the expected length of the incoming command. For now just turnong on/off so only need 1 bytes
#define COMMAND_LEN 1

int processCommand( const char *commandString ) {

  // Use the first byte to specify action...
 
  switch (commandString[0]) {
      
    case '0': 
     pinMode( LED_PIN, OUTPUT );
     p.runShellCommandAsynchronously("killall madplay");
     return 0;
          
    case '1':
     return 1;

    case '2':
     // neopixel_flicker();
     return 2;
//     break;    
     
    case '3':     
     p.runShellCommandAsynchronously("killall madplay");
     delay(5);
     // neopixel_party();
     delay(5);
     return 3;

//     break;

    case '4':
     // victory_routine();
//     digitalWrite( LED_PIN, LOW);
     return 4;     
     }
         
}


char commandBuffer[COMMAND_LEN];

void loop() {
//  Serial.println(usecase);
  
  // Does this look like a command request?  
  if (Serial1.find(COMMAND_PREFIX)) { 
    // Read it into the command buffer and only process if long enough...
    if ( Serial1.readBytes( commandBuffer , COMMAND_LEN ) == COMMAND_LEN ) {
  
      processCommand( commandBuffer );
      usecase = max(usecase,processCommand( commandBuffer ));
       
    }    
  }

  int reading = digitalRead(buttonPin);
  if (reading != lastReading){
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime)>debounceDelay){
    if(reading != buttonState){
      buttonState = reading;
      justToggled = 1;
    }
  }

  if (justToggled == 1){
    if (usecase > 0){
      usecase = 0;
      p.runShellCommandAsynchronously("killall madplay");
      firsttimeFlicker = 1;
      firsttimeParty = 1;
    }
    else {
      usecase = 1; 
    }
  }
  
  if (usecase == 0) {
      digitalWrite(magPin,HIGH); //dark mode
      lightsoff();
  }
  else if (usecase == 1){
      digitalWrite(magPin,HIGH); //lamp mode
      lightson();
  }
  else if (usecase == 2){ //flicker mode
      flicker();
      if(firsttimeFlicker == 1){
        p.runShellCommandAsynchronously("madplay --downsample -r -a -18dB /www/sd/test/images/electricity.mp3");
        firsttimeFlicker = 0;
      }
      digitalWrite(magPin,HIGH);
  }
  else if (usecase == 3) { //party mode
      party();
      if(firsttimeParty == 1){
        p.runShellCommandAsynchronously("madplay --downsample -a -12dB /www/sd/test/images/scramble_audio.mp3");
        firsttimeParty = 0;
      }
      digitalWrite(magPin,HIGH);
  }
  else if (usecase == 4) { //sparkle party
      party();
      treasure();
      digitalWrite(magPin, LOW);
  }

  lastReading = reading;
  justToggled = 0;
  delay(5);
}

void lightson() {
  for (int i=0; i<NUM_LEDSA1; i++){
    stripBig.setPixelColor(i, stripBig.Color(155,90,25));
  }
  stripBig.show();
  for (int i=0; i<NUM_LEDSA0; i++){
    stripSmall.setPixelColor(i, stripSmall.Color(0,0,0));
  }
  stripSmall.show();
}

void lightsoff() {
  for (int i=0; i<NUM_LEDSA1; i++){
    stripBig.setPixelColor(i, stripBig.Color(0,0,0));
  }
  stripBig.show();
  for (int i=0; i<NUM_LEDSA0; i++){
    stripSmall.setPixelColor(i, stripSmall.Color(0,0,0));
  }
  stripSmall.show();
}

void flicker() {
  int flick = random(0,3);
  for (int i=0; i<NUM_LEDSA1; i++){
    stripBig.setPixelColor(i, stripBig.Color(28,23,17));
    if (flick == 2 && i%3 == 0){
      stripBig.setPixelColor(i, stripBig.Color(200,200,200));
    }
  }
  stripBig.show();
  delay(45); //changed 100 to 45
  for (int i=0; i<NUM_LEDSA1; i++){
    stripBig.setPixelColor(i, stripBig.Color(28,23,17));
  }
  stripBig.show();
}

void party() {
  uint16_t i;
  unsigned long j=millis()%256;
  
    for(i=0; i<stripBig.numPixels(); i++) {
      stripBig.setPixelColor(i, Wheel(((4 * i * 256 / stripBig.numPixels()) + j) & 255));
    }
    stripBig.show();
  
}

void treasure() {
  for (int i=0; i<NUM_LEDSA0; i++){
    stripSmall.setPixelColor(i, stripSmall.Color(160,100,10));
  }
  stripSmall.show();
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return stripBig.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return stripBig.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return stripBig.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
