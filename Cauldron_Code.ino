/* CAULDRON DEMO
 */

#include <Wire.h>
#include <Adafruit_MPRLS.h>
#include <SPI.h>
#include <MFRC522.h>

// outputs to neopixel controller
#define COALS_PIN 3 //makes coals shimmer
#define BUBBLES_PIN 4 //makes cauldron bubble
#define FLUSH_PIN 5 //makes flush routine happen

// lever variables
#define LEVER_PIN 2
long leverStartTime = 0;
long leverElapsed = 0;
bool leverActive = 0;
long leverLockout = 7000;

// herb-related
const uint8_t herbPins[] = {6,7,8}; //ones, twos, fours for binary codes
const uint8_t num_herbPins = 3;
int herbIndex = 0; //used in some loops
uint8_t herbState[] = {0,0,0,0,0,0};
uint8_t numHerbsUsed = 0;
int sleepState = 6;
int victoryState = 7;
uint8_t herb = 0;
bool unused = 0;
bool used = 1;
byte numHerbs = 6;
int firstTime = 1; //for creating a delay when herbs are first input
int FINALE_DELAY = 8000;
/* Herb Pins Binary Guide:
 * 0 = listening
 * 1 = herb is 1
 * 2 = herb is 2
 * 3 = herb is 3
 * 4 = herb is 4
 * 5 = herb is 5
 * 6 = herb is 6
 * 7 = asleep
 * 8 = victory
 */
 
// crystal reading
#define CRYSTALS_PIN 17 //A3 = shared_complete
const byte crystalPins[] = {14,15,16}; //A0 = Air, A1 = Fire, A2 = Water
const byte numCrystals = 3;
bool crystalStatus[] = {0,0,0}; //Air, Fire, Water. (Alphabetical)
bool crystalPass = 0; //Pass or fail, default fail

// pressure sensing variables
#define RESET_PIN -1
#define EOC_PIN -1
long bellowsStartTime = 0;
long bellowsDebounce = 1000; //bellows debounce in ms
long bellowsElapsed = 0;
int diffP = 0;
uint16_t bellowsVal = 0; //used for reading bellows
const uint16_t BELLOWS_THRESHOLD = 15;
bool bellowsPresent = 1;
Adafruit_MPRLS bellows = Adafruit_MPRLS(RESET_PIN, EOC_PIN); // Create pressure sense instance
bool bBellowsOn = 0;
int newBellows = 0;

// RFID stuff
#define RST_PIN 9
#define SS_PIN 10
uint8_t goodCard[6][7]={
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // gray
  {0x04,0x79,0x83,0x2A,0x09,0x61,0x81}, // red
  {0x04,0x30,0x88,0x2A,0x09,0x61,0x80}, // green
  {0x04,0x3A,0x88,0x2A,0x09,0x61,0x80}, // blue
  {0x04,0x05,0x89,0x2A,0x09,0x61,0x81}, // yellow
  {0x04,0x5B,0x8E,0x2A,0x09,0x61,0x81}, // magenta
};
bool rfidPass = 1;
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
uint8_t UID[7]; //buffer
byte tagSize = 7;


int bellowsBaseline = 0;

void setup() {
  delay(2000); //stabilize

  Serial.begin(9600); 
  if(! bellows.begin()){ //init pressure sensor
    bellowsPresent = 0;
  }
  else {
    bellowsPresent = 1;
    for(int i=0;i<20;i++){
      bellowsBaseline = bellowsBaseline + bellows.readPressure();
      delay(100);
    }
    bellowsBaseline = bellowsBaseline / 20;
  }
  SPI.begin(); // init SPI bus
  mfrc522.PCD_Init(); //init RFID reader

  
  pinMode(LEVER_PIN, INPUT_PULLUP);

  //set outputs HIGH for active low logic
  pinMode(COALS_PIN, OUTPUT);
  digitalWrite(COALS_PIN, HIGH);
  
  pinMode(BUBBLES_PIN, OUTPUT);
  digitalWrite(BUBBLES_PIN, HIGH);

  pinMode(FLUSH_PIN, OUTPUT);
  digitalWrite(FLUSH_PIN, HIGH);

  pinMode(CRYSTALS_PIN, OUTPUT);
  digitalWrite(CRYSTALS_PIN, HIGH);
  
  for(int i=0;i<num_herbPins;i++){
    pinMode(herbPins[i], OUTPUT);
    digitalWrite(herbPins[i],HIGH);
  }

  for(byte i=0;i<numCrystals; i++){
    pinMode(crystalPins[i],INPUT_PULLUP);
  }

  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max); 
}

void loop() {
  if(millis() % 1000 == 0){
    
    newBellows = bellows.readPressure();
    diffP = abs(newBellows - bellowsBaseline);
    
    if(diffP < BELLOWS_THRESHOLD){
      bellowsBaseline = (bellowsBaseline * 19 + newBellows)/20;
    }
    
  }
  
  //Bellows is always on
  bool bBellowsOn = bellowsOn();
  
  bellowsElapsed = millis() - bellowsStartTime;
  leverElapsed = millis() - leverStartTime;

  if(bBellowsOn){
    bellowsStartTime = millis();
  }
  
  if(bBellowsOn || bellowsElapsed < bellowsDebounce){
    digitalWrite(COALS_PIN, LOW); //turn coals on
    if(leverElapsed > leverLockout) {
      leverActive = 1;
    }
    if(crystalsEverSolved()){
      digitalWrite(BUBBLES_PIN, LOW); //turn bubbles on
    }
  }
  else {
    digitalWrite(COALS_PIN, HIGH); //turn coals off
    digitalWrite(BUBBLES_PIN, HIGH); //turn bubbles off
    leverActive = 0;
  }

  if(crystalsEverSolved()){
    
    herb = rfidRead();
    
    if(herb==0){
      setHerbReader(herb); //Herb reader default "awake" state
    }
    
    else{
      if(herbState[herb] == unused && leverElapsed > leverLockout) {
        setHerbReader(herb);
        if(leverPull() && leverActive) {
          leverStartTime = millis();
          flusher();
          herbState[herb] = used;
          numHerbsUsed++;
        }
      }
    }

    herbState[0] = unused;
    
    if(numHerbsUsed == 4){
      if(firstTime == 1) {
        delay(FINALE_DELAY);
        firstTime = 0;
      }
      setHerbReader(victoryState); //Send Cauldron into Victory routine
    }
  }

  else {
    setHerbReader(sleepState); //Herb reader "asleep" state
  }
  delay(5);
}

bool bellowsOn(){
  if (bellowsPresent == 0){
    return 1;
  }
  else {
    bellowsVal = bellows.readPressure();    
    int dDiffP = abs(bellowsVal - bellowsBaseline);

    if(dDiffP > BELLOWS_THRESHOLD){
      delay(70);
      
      bellowsVal = bellows.readPressure();
      dDiffP = abs(bellowsVal - bellowsBaseline);
      
      if(dDiffP > BELLOWS_THRESHOLD){
        return 1;
      }
      
    }
    else {
      return 0;
    }
  }
}

bool crystalsEverSolved(){     
  Serial.print("Air/Fire/Water = ");
  
  if(!crystalPass){
    crystalPass = 1;
    
    for(byte i=0; i<numCrystals; i++){
      crystalStatus[i] = ! digitalRead(crystalPins[i]); //negated for INPUT_PULLUP

      if(! crystalStatus[i]){
        crystalPass = 0;
      }
      Serial.print(crystalStatus[i]);
    }
  }
  
//  crystalPass = 1; //FOR DEBUG, THIS IS PERMANENTLY "SOLVED"

  Serial.print(" Shared Complete = ");
  Serial.print(!crystalPass);
  Serial.println();
  digitalWrite(CRYSTALS_PIN, ! crystalPass);
  
  return crystalPass;
  
}

int rfidRead(){
  herbIndex = 0;
  
  //initialize UID
  for(int i=0;i<tagSize;i++){
    UID[i] = 0x00;
  }

  // Read UID twice
  if(mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    for(int i=0;i<mfrc522.uid.size;i++){
      UID[i] = mfrc522.uid.uidByte[i];
    }
  }
  
  if(mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    for(int i=0;i<mfrc522.uid.size;i++){
      UID[i] = mfrc522.uid.uidByte[i];
    }
  }

  //check which RFID tag
  for(int j=0;j<numHerbs;j++){
    rfidPass = 1;
    for(int i=0;i<tagSize;i++){
      if(UID[i] != goodCard[j][i]){
        rfidPass = 0;
      }
    }
    if (rfidPass == 1){
      herbIndex = j;
    }
  }
  return herbIndex;
}

// tells herb base what color to glow
void setHerbReader(int hHerb){
  for(int i=0;i<3;i++){
    digitalWrite(herbPins[i],1-bitRead(hHerb,i)); //input_pullup logic
  }
}

bool leverPull(){
  bool leverState = digitalRead(LEVER_PIN);
  if(!leverState){
    delay(2);
    leverState = digitalRead(LEVER_PIN);
  }
  return !leverState;
}

void flusher(){  
  digitalWrite(FLUSH_PIN, LOW);
  delay(200);
  digitalWrite(FLUSH_PIN, HIGH);
}
