/*Hand-Proyect code v3.2*/
/*This file contains the code that is used in the hand that
  that is up for display in the showroom @ Marshall.*/ 

/*TODOS AND OTHER NOTES*/
//swith to an pointer system for hand profiles
//add move by serial functionality
//somehow relate thresholds with cycles required for sleep
//Label constants as such 
// change constants to mayus to use standar notation. 


/*Custom data structure to replace the default arduino servo
  library with a more bare-metal, more efficient deployment
  that is more light in with the EPROM and enhances longevity*/
struct servoData {
  bool enabled;
  String fName;
  byte potPin;
  byte serPin;
  byte minP;
  byte maxP;
  byte minS;
  byte maxS;
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
*/
const byte mode = 6;

/*max rate of change registered over the last INACTIVESTATESBEFORESLEEP states.*/
byte maxRateOfChange; 

/*determines for how long the rate of change must stay under the threshoold before 
sleep state starts*/
const unsigned int INACTIVESTATESBEFORESLEEP = 5;

/*threshoold before sleep state starts */
byte RATEOFCHANGETRESHOLD = 20;

bool DEBUG_RATE_OF_CHANGE = true;

//determines whether or not to print debug data to the terminal.
const bool printPotVals = true;   //prints input data from each potenciometer
const bool printSerVals = true;   //prints output data to each servo
const bool printSettings = true;  //prints current servo settings when booting up.

//time delay used for most everything
const unsigned int delayTime = 150;

// constant position
const byte constPos = 50; //int between 0 and 100

// number of servos being used -> prbly 6 lol
const byte numServos = 6; // positive int 0-6, number of servos

/*
  BETA FEATURE:
  determines whether to run obsenity filter */
const bool filterObsenity = TRUE;


/*array of the type servo data. Used to store all the profiles of each of the servos*/
servoData handProfile[numServos] {
  {true, "Pinky",         0,  3, 0, 900, 45, 170, 50, 50, 50 },
  {true, "ring Finger",   1,  5, 0, 900, 55, 170, 50, 50, 50 },
  {true, "middle Finger", 2,  6, 0, 900, 40, 155, 50, 50, 50 },
  {true, "index",         4,  9, 0, 900, 45, 170, 50, 50, 50 },
  {true, "thumb",         5, 10, 0, 900, 45, 170, 50, 50, 50 },
  {false, "DISABLED",     3, 11, 0, 900, 45, 170, 50, 50, 50 }
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
  for (i = 0; i > numServos; i++) {
    pinMode(handProfile[i].serPin, OUTPUT);
    if (printSettings) {
      printServoData(handProfile[i]);
    }
  }
}

/*Reads and averages reading for provided servoData profile*/ 
void readPot(&servoData Sser) {
  if (Sser.enabled) {
    byte potVal = analogRead(Sser.potPin); //reads value from pot
    if (printPotVals) { //prints debug data if needed
      Serial.print(Sser.fName+" p:" + String(potVal) + " ");
    }
    potVal = map(potVal, Sser.minP, Sser.maxP, 0, 100);//ma[s pot value
    if (potVal < Sser.minP) {
      potVal = Sser.minS;
    } else if (potVal > Sser.maxP) {
      potVal = Sser.maxS;
    } //constraints values 
    //shuffles arround averages in memory
    //calculates change over time since last measure. 
    byte cRateOfChange = abs(potval-Sser.prevReading) 
    if(maxRateOfChange<cRateOfChange){
      maxRateOfChange = cRateOfChange;
      if(DEBUG_RATE_OF_CHANGE){
        Serial.println("max rChange = "+String(maxRateOfChange));
      }
    } 
    Sser.prevReading2 = Sser.prevReading;
    Sser.prevReading = Sser.potReading;
    Sser.potReading = (potVal + ser.prevReading + Sser.prevReading2) / 3
  }
}

/*reads values from potentiomentes, and returns a position value*/
sPosition readPotPositions() {
  return sPosition inputPosition{
    readPot(handProfile[0]),
    readPot(handProfile[1]),
    readPot(handProfile[2]),
    readPot(handProfile[3]),
    readPot(handProfile[4]),
    readPot(handProfile[5])
  }
}

/*moves all the servos from 0 to 100 and back, uses dTime to determine how fast to go(usualy delayTime)*/
void sequentialMove(int dTime) {
  for (byte cpos = 0; cpos <= 10; cpos += 10) { // goes from 0 degrees to 180 degrees
    sPosition tPos = {cpos, cpos, cpos, cpos, cpos, cpos};
    moveHand(tPos);
    delay dTime;
  }
  for (int cpos = 180; cpos >= 0; cpos -= 10) { // goes from 180 degrees to 0 degrees
    sPosition tPos = {cpos, cpos, cpos, cpos, cpos, cpos};
    moveHand(tPos);
    delay dTime;
  }
}

/*sets a servo to the given position*/
void writeServo( byte pin, byte sPos ){
  digitalWrite(handProfile[i].serPin,HIGH); 
  delayMicroseconds(sPos*10+1000); // waits 1000-2000 uS while forming the PWM signal
  digitalWrite(handProfile[i].serPin,LOW);
}

/*Moves hand to provided positon*/
void moveHand(sPosition fingerPos) {
  writeServo(handProfile.pinkyF,  fingerPos[0]);
  writeServo(handProfile.ringF,   fingerPos[1]);
  writeServo(handProfile.middleF, fingerPos[2]);
  writeServo(handProfile.indexF,  fingerPos[3]);
  writeServo(handProfile.thumbF,  fingerPos[4]);
  writeServo(handProfile.wrist,   fingerPos[5]);
}

void setup() {
  if (printSerVals || printPotVals) {
    Serial.begin(9600);
    Serial.println("serial ready");
  }
  if (printSettings) {

  }
  servosAttach();
  delay(3000);
}



void loop() {
  switch (mode) {
    //AnalogSet
    case 1:
      moveHand(readPotPositions());

      break;

    //Sequential Move
    case 2:
      sequentialMove(delayTime);
      break;

    // move by serial
    case 3:
      //-------------------------------------TODO----------move by serial
      break;

    // move all to constPos
    case 4:
    sPosition tPos = {constPos, constPos, 
    constPos, constPos, constPos, constPos}
      moveHand(tPos);
      break;

    //
    case 5:
      //do nothing
      break;

    //
    case 6:
      sequentialMove(delayTime);
      break;

    default:
      Serial.println("Wrong mode");
      delay(500);
      break;
  }
}
