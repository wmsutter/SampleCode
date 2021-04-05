/*WATER PUMP
 */
 
int trigPin = 3;    // Trigger
int echoPin = 4;    // Echo
int IRLedPin = 5; // Crystal charging

long duration, mmNow, mmOld, velocity;

bool moving = 0;
bool pumping = 0;
long lastPumped = 0; 
byte pumpingSamples = 0;

int pumpLag = 1000; // how long to hold pump on after pumping
int samplingHz = 10; //for rangefinding
byte debounceNumber = 6; //for velocity debounce
int velocityThreshold = 5; //cm per sec

long lastMillis = 0;
 
void setup() {
  //Serial Port begin
  Serial.begin (9600);
  //Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(IRLedPin, OUTPUT);
}
 
void loop() {
  if(millis() - lastMillis > (1000/samplingHz)){
    duration = sample();
    mmNow = 10 * duration / 2 / 29.1; //convert time to dist
    velocity = (mmNow - mmOld) * samplingHz / 10; //convert to cm/s
    lastMillis = millis();
    mmOld = mmNow;
  }

  if(abs(velocity) >= velocityThreshold && pumpingSamples < debounceNumber) {
    pumpingSamples++;
  }
  else{
    pumpingSamples = 0;
  }

  if(pumpingSamples == debounceNumber){
    lastPumped = millis();
  }

  (millis() - lastPumped <= pumpLag) ? pumping = true : pumping = false;

  if(pumping){
    digitalWrite(IRLedPin, LOW);
  }
  else{
    digitalWrite(IRLedPin, HIGH);
  }
  
  Serial.print("dist = ");
  Serial.print(mmNow);
  Serial.print("mm");
  Serial.print("\tvel = ");
  Serial.print(velocity);
  Serial.print("cm/s");
  Serial.print("\tPumping is ");
  Serial.print((pumping) ? "ON" : "OFF");
  Serial.println();

}

int sample() {
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  long aDuration = pulseIn(echoPin, HIGH, 2000);
  return aDuration;
}
