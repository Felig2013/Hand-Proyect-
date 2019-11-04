/*Hand-Proyect code v3.2*/
/*This file contains the code that is used in the hand that
  that is up for display in the showroom @ Marshall.*/ 

/*TODOS AND OTHER NOTES*/
//swith to an pointer system for hand profiles
//add move by serial functionality
//somehow relate thresholds with cycles required for sleep
//Label constants as such 
// change constants to mayus to use standar notation. 
// i like github
#include "header.h"


/*Custom data structure, used to store a particular position for
  the hand.
  uses PERCENTAGES not Angles to define positions*/
struct sPosition {
  byte pinkyF;
  byte ringF;
  byte middleF;
  byte indexF;
  byte thumbF;
  byte wrist;
};

/*--------Set this varible to choose the mode the arduion will be in----------

  mode 1:
    read data from 10k pot and move servos acordingly.
  mode 2: 
    secuential move (for debug and calibration)
  mode 3: 
    move by serial (NOT READY)
  mode 4: 
    move all to constPos (move all servos to constPos and leave them there)
  mode 5:
    do nothing
  mode 6: 
    sequential move
  mode 7: 
    sleep mode
  
*/
char mode = '1';

/*max rate of change registered over the last INACTIVESTATESBEFORESLEEP states.*/
byte maxRateOfChange; 

/*determines for how long the rate of change must stay under the threshoold before 
sleep state starts*/
const unsigned int INACTIVESTATESBEFORESLEEP = 5;

/*threshoold before sleep state starts */
const byte RATEOFCHANGETRESHOLD = 20;

const bool DEBUG_RATE_OF_CHANGE = true;

//determines whether or not to print debug data to the terminal.
const bool printPotMapping = 0;
const bool printPotVals    = 0;   //prints input data from each potenciometer
const bool printSerVals    = 1;   //prints output data to each servo
const bool printSettings   = 0;   //prints current servo settings when booting up.

//time delay used for most everything
const unsigned int delayTime = 500;

// constant position
const byte constPos = 50; //int between 0 and 100

// number of servos being used -> prbly 6 lol
const byte numServos = 6; // positive int 0-6, number of servos

/*
  BETA FEATURE:
  determines whether to run obsenity filter */
const bool filterObsenity = true;


/*array of the type servo data. Used to store all the profiles of each of the servos*/
servoData handProfile[numServos] {
  {1, "Pinky",          0,  3, 0, 900, 10, 90, 50, 50, 50 },
  {1, "ring Finger",   1,  5, 0, 900, 10, 90, 50, 50, 50 },
  {1, "middle Finger", 2,  6, 0, 900, 10, 90, 50, 50, 50 },
  {1, "index",         4,  9, 0, 900, 10, 90, 50, 50, 50 },
  {1, "thumb",         5, 10, 0, 900, 10, 90, 50, 50, 50 },
  {0, "DISABLED",      3, 11, 0, 900, 10, 90, 50, 50, 50 }
};

/*predefined positions*/
sPosition numberOne {0, 0,  0, 90, 90};
sPosition peace     {0, 0, 90, 90, 90};

/*prints and formats data from parameter servoData*/
void printServoData(servoData servoSetting) {
  Serial.println("Attached "+servoSetting.fName
  +" PRange: "+String(servoSetting.minP)+"-"+String(servoSetting.maxP)
  +" SRange: "+String(servoSetting.minS)+"-"+String(servoSetting.maxS)
  +" sPin: "+String(servoSetting.serPin)+" pPin: "+String(servoSetting.potPin) );
}

/*Attaches all the servos to the pins they are connected to and
  prints debug data if flag is set*/
void servosAttach() {
  for (int i = 0; i > numServos; i++) {
    pinMode(handProfile[i].serPin, OUTPUT);
    if (printSettings) {
      printServoData(handProfile[i]);
    }
  }
}

/*Reads and averages reading for provided servoData profile*/ 
byte readPotVal(servoData &Sser) {
  if (Sser.enabled) {
    int potVal = analogRead(Sser.potPin); //reads value from pot
    if (printPotVals) { //prints debug data if needed
      Serial.print(Sser.fName+" p:" + String(potVal) + " ");
    }
    potVal = map(potVal, Sser.minP, Sser.maxP, 0, 100);//map pot value
    if (printPotMapping) { //prints debug data if needed
      Serial.print("pM:" + String(potVal) + " ");
    }
    if (potVal < Sser.minS) {//constraints values 
      potVal = Sser.minS;
    } else if (potVal > Sser.maxS) {
      potVal = Sser.maxS;
    }
    if (printPotMapping) { //prints debug data if needed
      Serial.print("pC:" + String(potVal) + " ");
    }
    //calculates change over time since last measure. 
    byte cRateOfChange = abs(Sser.potReading-Sser.prevReading);
    if(maxRateOfChange<cRateOfChange){
      maxRateOfChange = cRateOfChange;
      if(DEBUG_RATE_OF_CHANGE){
        Serial.println("max rChange = "+String(maxRateOfChange));
      }
    } 
    //shuffles arround averages in memory
    Sser.prevReading2 = Sser.prevReading;
    Sser.prevReading = Sser.potReading;
    Sser.potReading = (potVal + Sser.prevReading + Sser.prevReading2) / 3;
   
    if (printPotMapping) { //prints debug data if needed
      Serial.print("pO:" + String(Sser.potReading) + " "+"p(" + String(Sser.potReading) + " " + String(Sser.prevReading)+ " " + String(Sser.prevReading2) +") ");
    }
  }
  return Sser.potReading;
}

/*reads values from potentiomentes, and returns a position value*/
sPosition readPotValPositions() {
  sPosition inputPosition{
    readPotVal(handProfile[0]),
    readPotVal(handProfile[1]),
    readPotVal(handProfile[2]),
    readPotVal(handProfile[3]),
    readPotVal(handProfile[4]),
    readPotVal(handProfile[5])
  };
  Serial.println();
  return inputPosition;
}

/*moves all the servos from 0 to 100 and back, uses dTime to determine how fast to go(usualy delayTime)*/
void sequentialMove(int dTime) {
  for (byte cpos = 0; cpos <= 10; cpos += 10) { // goes from 0 degrees to 180 degrees
    sPosition tPos = {cpos, cpos, cpos, cpos, cpos, cpos};
    moveHand(tPos);
    delay (dTime);
  }
  for (int cpos = 180; cpos >= 0; cpos -= 10) { // goes from 180 degrees to 0 degrees
    sPosition tPos = {cpos, cpos, cpos, cpos, cpos, cpos};
    moveHand(tPos);
    delay (dTime);
  }
}

/*sets a servo to the given position*/
void writeServo( byte sPos, servoData S){
  if (printSerVals) { //prints debug data if needed
      Serial.print("S:" + String(sPos) + " ");
  }
  int dTime = map(sPos, 0, 100, S.minS, S.maxS )+1000;
  digitalWrite(S.serPin,HIGH); 
  delayMicroseconds(dTime); // waits 1000-2000 uS while forming the PWM signal
  digitalWrite(S.serPin,LOW);
}

/*Moves the hand to the desired sPosition*/
void moveHand(sPosition fingerPos){
writeServo(fingerPos.pinkyF,  handProfile[0]);
writeServo(fingerPos.ringF,   handProfile[1]);
writeServo(fingerPos.middleF, handProfile[2]);
writeServo(fingerPos.indexF,  handProfile[3]);
writeServo(fingerPos.thumbF,  handProfile[4]);
writeServo(fingerPos.wrist,   handProfile[5]);
Serial.println();
}


void setup() {
  if (printSerVals || printPotVals || printSettings) { //if things need to get
    Serial.begin(9600);
    Serial.println("serial ready");
  }
  servosAttach();
  delay(1000);
}

void loop() {
  sPosition tPos;
  switch (mode) {
    //AnalogSet
    case '1':
      moveHand(readPotValPositions());

      break;

    //Sequential Move
    case '2':
      sequentialMove(delayTime);
      break;

    // move by serial
    case '3':
      //-------------------------------------TODO----------move by serial
      break;

    // move all to constPos
    case '4':
    tPos = {constPos, constPos, 
    constPos, constPos, constPos, constPos};
      moveHand(tPos);
      break;

    //
    case '5':
      //do nothing
      break;

    //
    case '6':
      sequentialMove(delayTime);
      break;
    case '7':
      tPos = readPotValPositions();
      if(filterObsenity){
        //obsenity check 
      }
      moveHand(tPos);
      while (true){
        readPotValPositions(); //also updates rate of change
        if(maxRateOfChange>RATEOFCHANGETRESHOLD){
          mode = 1;
          if(DEBUG_RATE_OF_CHANGE){
            Serial.println("out of sleep; rChange = "+String(maxRateOfChange));
          }
          break;
        }
      }
      break;
    default:
      Serial.println("Wrong mode");
      delay(500);
      break;
  }
}
