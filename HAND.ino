/*Hand-Proyect code v4.0*/
/*This file contains the code that is used in the hand that
  that is up for display in the showroom @ Marshall.*/ 

/*TODOS AND OTHER NOTES*/
//swith to an pointer system for hand profiles
//add move by serial functionality
//somehow relate thresholds with cycles required for sleep
//Label constants as such 
// change constants to mayus to use standar notation. 


/*data about a servo*/
struct servoData {
  String fName;
  byte serPin;
  int minS;
  int maxS;
};

/*data about a potenciometer*/
struct potData {
  byte potPin;
  int minP;
  int maxP;
  byte potReading;
  byte prevReading;
  byte prevReading2;
};
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
  mode 8: 
    serial report servo 
  mode 9: 
    serial report servo 
*/ char MODE = '1';

/*max rate of change registered over the last INACTIVESTATESBEFORESLEEP states.*/
byte maxRateOfChange; 

/*determines for how long the rate of change must stay under the threshoold before 
sleep state starts*/
const unsigned int INACTIVESTATESBEFORESLEEP = 5;

/*threshoold before sleep state starts */
const byte RATEOFCHANGETRESHOLD = 20;

const bool DEBUG_RATE_OF_CHANGE = true;

//determines whether or not to print debug data to the terminal.
const bool PRINTPOTVALPROCESSING = 0; //Prints the intermediate steps in readPotVal
const bool PRINTPOTVALS    = 1;   //prints input data from each potenciometer
const bool PRINTSERVALS    = 1;   //prints output data to each servo
const bool PRINTSERTIMES   = 1;
//time delay used for most everything
const unsigned int DELAYTIME = 300;

// constant position
const byte CONSTPOS = 50; //int between 0 and 100
//
const byte REFRESHESPERCYCLE = 4;
const byte DELAYBETWENREFRESHES = 100;

/*
  BETA FEATURE:
  determines whether to run obsenity filter */
const bool FILTEROBSENITY = true;

/*number of servos */
const byte NUMSERVOS = 6;

potData readerProfile[6] {
  {0, 0, 900, 50, 50, 50 },
  {1, 0, 900, 50, 50, 50 },
  {2, 0, 900, 50, 50, 50 },
  {4, 0, 900, 50, 50, 50 },
  {5, 0, 900, 50, 50, 50 },
  {3, 0, 900, 50, 50, 50 }
};

servoData handProfile[6] {
  {"Pinky",         3,  100, 900},
  {"ring Finger",   5,  100, 900},
  {"middle Finger", 6,  100, 900},
  {"index",         9,  100, 900},
  {"thumb",         10, 100, 900},
  {"DISABLED",      11, 100, 900}
};

/*predefined positions*/
sPosition NUMBERONE {0, 0,  0, 90, 90};
sPosition PEACE     {0, 0, 90, 90, 90};

/*Attaches all the servos to the pins they are connected*/
void servosAttach() {
  for (int i = 0; i > NUMSERVOS; i++) {
    pinMode(handProfile[i].serPin, OUTPUT);
  }
}
/*Attaches all the potenciometers to the pins they are connected to.*/
void potsAttach() {
  for (int i = 0; i > NUMSERVOS; i++) {
    pinMode(readerProfile[i].potPin, INPUT);
  }
}

/*Reads and averages reading for provided servoData profile*/ 
byte readPotVal(potData &pData) {
    int potVal = analogRead(pData.potPin); //reads value from pot
    if (PRINTPOTVALS) { //prints debug data if needed
      Serial.print(String(pData.potPin)+" p:" + String(potVal) + " ");
    }
    potVal = map(potVal, pData.minP, pData.maxP, 0, 100);//map pot value
    if (PRINTPOTVALPROCESSING) { //prints debug data if needed
      Serial.print("pM:" + String(potVal) + " ");
    }
    if (potVal <  0) {//constraints values 
      potVal = 0;
    } else if (potVal > 100) {
      potVal = 100;
    }
    if (PRINTPOTVALPROCESSING) { //prints debug data if needed
      Serial.print("pC:" + String(potVal) + " ");
    }
    //calculates change over time since last measure. 
    byte cRateOfChange = abs(pData.potReading-pData.prevReading);
    if(maxRateOfChange<cRateOfChange){
      maxRateOfChange = cRateOfChange;
      if(DEBUG_RATE_OF_CHANGE){
        Serial.println("max rChange = "+String(maxRateOfChange));
      }
    }
    //shuffles arround averages in memory
    pData.prevReading2 = pData.prevReading;
    pData.prevReading = pData.potReading;
    pData.potReading = (potVal + pData.prevReading + pData.prevReading2) / 3;
   
    if (PRINTPOTVALPROCESSING) { //prints debug data if needed
      Serial.print("pO:" + String(pData.potReading) + " "+"p(" + String(pData.potReading) + " " + String(pData.prevReading)+ " " + String(pData.prevReading2) +") ");
    }
  
  return pData.potReading;
}

/*reads values from potentiomentes, and returns a position value*/
sPosition readPotValPositions() {
  sPosition inputPosition{
    readPotVal(readerProfile[0]),
    readPotVal(readerProfile[1]),
    readPotVal(readerProfile[2]),
    readPotVal(readerProfile[3]),
    readPotVal(readerProfile[4]),
    readPotVal(readerProfile[5])
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
  if (PRINTSERVALS) { //prints debug data if needed
      Serial.print("S:" + String(sPos) + " ");
  }
  int dTime = map(sPos, 0, 100, S.minS, S.maxS )+1000;
  if (PRINTSERTIMES) { //prints debug data if needed
      Serial.print("dT:" + String(dTime) + ", "+ String(S.serPin) + " ");
  }
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
  if (true) { //if things need to get
    Serial.begin(9600);
    Serial.println("serial ready");
  }
  servosAttach();
  delay(1000);
}

void loop() {
  sPosition tPos;
  delay(DELAYTIME);
  switch (MODE) {
    //AnalogSet
    case '1':
      for (byte i = 0; i < REFRESHESPERCYCLE; i++){
      delay(DELAYBETWENREFRESHES);
      readPotValPositions();
      }
      moveHand(readPotValPositions());
      break;

    //Sequential Move
    case '2':
      sequentialMove(DELAYTIME);
      break;

    // move by serial
    case '3':
      //-------------------------------------TODO----------move by serial
      break;

    // move all to constPos
    case '4':
      tPos = {CONSTPOS, CONSTPOS, CONSTPOS, CONSTPOS, CONSTPOS, CONSTPOS};
      moveHand(tPos);
      break;

    //
    case '5':
      //do nothing
      break;

    //
    case '6':
      sequentialMove(DELAYTIME);
      break;

    case '7':
      tPos = readPotValPositions();
      if(FILTEROBSENITY){
        //obsenity check 
      }
      moveHand(tPos);
      while (true){
        readPotValPositions(); //also updates rate of change
        if(maxRateOfChange>RATEOFCHANGETRESHOLD){
          MODE = 1;
          if(DEBUG_RATE_OF_CHANGE){
            Serial.println("out of sleep; rChange = "+String(maxRateOfChange));
          }
          break;
        }
      }
      break;
      
    case '8':
      readPotVal(readerProfile[0]);
      break;
    default:
      Serial.println("Wrong mode");
      delay(500);
      break;
  }
}
