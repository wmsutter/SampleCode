/**
 * Shifting Portrait, gpio edition: 
 * 
 * On key press, check which key it was and fade in the right image.
 * If the victory key is pressed, fade in pre-wolf and then post-wolf.
 * If the reset key is pressed, reset.
 * 
 */

import ddf.minim.*;
import processing.io.*;

Minim minim;
AudioPlayer sparkle;
AudioPlayer growl;


int numImages = 15;
//int keys[] = {1,2,3,4,5,6,7,8};
int k = 0; // base image index
int j = 0; // new image index
//int s = 0; // solve state index
PImage[] images = new PImage[numImages]; // image array

int fader = 0;
int fadeSpeed = 2;
boolean newSend = false; // for rising edge detection
boolean oldSend = false; // for rising edge detection
int pinArray[] = {7,8,4,5,6};
int numPins = 5;
int sendPin = pinArray[numPins-1];
int message = 0;
int oldMessage = 0;
int adder = 0;
int ones = 0;
int twos = 0;
int fours = 0;
int eights = 0;
int wolfIndex = 0;
int currentImageIndex = 0;
int lastImageIndex = 0;
boolean fadeOut = false;
boolean reading = false;
boolean lastReading = false;
boolean growlFlag = false;

int firstNine = 0;
int nineLockout = 5000;
int firstSend = 0;
int sendDebounce = 100;

void setup() {
  for(int i=0;i<numPins;i++){
    GPIO.pinMode(pinArray[i], GPIO.INPUT_PULLDOWN);
  }
 
  size(480, 720); // canvas size
  frameRate(30); // loop speed
  
  // Load sounds into the program  
  minim = new Minim(this);
  sparkle = minim.loadFile("sparkle.mp3");
  growl = minim.loadFile("growl.wav");
  
  sparkle.setGain(11);
  growl.setGain(4);
  
  // Load the images into the program
  images[0] = loadImage("default.jpg");
  images[1] = loadImage("Clara.jpg");
  images[2] = loadImage("David.jpg");
  images[3] = loadImage("Ginger.jpg");
  images[4] = loadImage("Jacob.jpg");
  images[5] = loadImage("Lawrence.jpg");
  images[6] = loadImage("Scott.jpg");
  images[7] = loadImage("Vera.jpg");
  images[8] = loadImage("Ellie.jpg");
  images[9] = loadImage("Werewolf.jpg");
  images[10] = loadImage("Cure.jpg");
  images[11] = loadImage("Kill.jpg");
  images[12] = loadImage("Fail.jpg");
  images[13] = loadImage("Party.jpg");
  images[14] = loadImage("Photobooth.jpg");
  
  for (int i=0; i<numImages; i++) {
    images[i].resize(480,720); // make all the images fit (360*2.75,540*3.4)
  }
}

void draw() {
  tint(255,255); // set base image to full opacity
  image(images[lastImageIndex],0,0); // display correct base image
  
  // check for send pin
  if (GPIO.digitalRead(sendPin) == GPIO.HIGH){
    reading = true;
  }
  else {
    reading = false;
  }
  
  if (reading != lastReading){
    firstSend = millis();
  }
  
  if (millis() - firstSend > sendDebounce) {
    if (reading != newSend){
      newSend = reading;
    }
  }
  
  // when there's an incoming message, decode it
  if (newSend){
    message = 0;
    for (int i = 0;i<(numPins-1);i++){
      if (GPIO.digitalRead(pinArray[i]) == GPIO.HIGH){
        adder = 1;
      }
      else {
        adder = 0;
      }
      message = message + (int(pow(2,i))*adder);
    }

    // if the message changed, get ready for some fading
    if(message != oldMessage){
      fader = 0;
      if(message == 9){ // if you're going to wolf, Ellie was last
        firstNine = millis(); 
      }
      else if(message > 9){ //if you're in party or photo mode, increment
        lastImageIndex = currentImageIndex;
        if (message == 12){
          growlFlag = true;
        }
      }
      else {lastImageIndex = 0;}
    }    
    
    // handle reset commands
    if (message == 0) {
      fader = 0;
      lastImageIndex = 0;
      wolfIndex = 0;
    }
    
    currentImageIndex = message;
    fadeOut = false;

    if(message == 9){
      if(millis() - firstNine < nineLockout){
        currentImageIndex = 8;
      }
      else if (wolfIndex == 0){
        lastImageIndex = 8;
        currentImageIndex = 8;
        wolfIndex = 1;
      }
      else if (wolfIndex == 1){
        fader = 0;
        wolfIndex = 2;
        growlFlag = true;
      }
    }    
  }
  
  else { // if NOT new send, fade out
    if(fader*fadeSpeed > 0) {
      fadeOut = true;
      fader--;
    }
  }
  
  if (newSend != oldSend || growlFlag){
    sparkle.pause();
    sparkle.rewind();
    growl.pause();
    growl.rewind();
    if (currentImageIndex == 9 || currentImageIndex == 12) {
      growl.play();
      growlFlag = false;
    }
    else {sparkle.play();}
  }
  
  // if new img isn't done fading
  if (fader * fadeSpeed < 255){      
    tint(255, fader*fadeSpeed);  // Set new image's opacity
    if(!fadeOut){ // and you aren't fading out
      fader++; // increment the fade
    }
  }

  image(images[currentImageIndex],0,0); //show the fading image
  
  oldMessage = message;
  oldSend = newSend;
  lastReading = reading;
  ones = 0;
  twos = 0;
  fours = 0;
  eights = 0;
}
