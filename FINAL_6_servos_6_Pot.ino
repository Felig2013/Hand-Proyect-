#include <Servo.h>
//mode 1: read data from 10k pot and move servos acordingly
//mode 2: secuential move 
//mode 3: move by serial (NOT READY)
//mode 4: move all to defined pos.
//mode 5: individual controll. (NOT READY)
//mode 6: sequential move

const char mode = 6;
const bool printPotVals = true;
const bool printSerVals = true;
const bool printSettings = true;
int delayTime = 150;
const byte constPos = 50; //int between 0 and 180

struct servoSetting{
  String fName;
  int potNum;
  int minP;
  int maxP;
  int minS;
  int maxS;
}; 
// Creates servo objects and creates setting variables.
Servo ser1;
servoSetting Sser1 = {"Pinky", 0, 0, 900, 45, 170 };
Servo ser2;
servoSetting Sser2 = {"ring Finger", 1, 0, 900, 55, 170 }; 
Servo ser3;
servoSetting Sser3 = {"middle Finger", 2, 0, 900, 40, 155 }; 
Servo ser4;
servoSetting Sser4 = {"DISABLED", 5, 0, 900, 45, 170 }; 
Servo ser5;
servoSetting Sser5 = {"Servo5", 4, 0, 900, 45, 170 }; 
Servo ser6;
servoSetting Sser6 = {"DISABLED", 3, 0, 900, 45, 170 }; 


// Attaches all the servos to the pins they are connected to. 
void servosAttach(){
  ser1.attach(3);
  ser2.attach(5);
  ser3.attach(6);
  ser4.attach(9);
  ser5.attach(10);
  ser6.attach(11);
  
  ser1.write(90);
  ser2.write(90);
  ser3.write(90);
  ser4.write(90);
  ser5.write(90);
  ser6.write(90);
}
// Maps a potentiomenter according to the 
int mapPot(servoSetting Sser){
  int potVal = analogRead(Sser.potNum);
  int serVal = map(potVal, Sser.minP, Sser.maxP, Sser.minS, Sser.maxS);
  if(serVal<Sser.minS){
    serVal=Sser.minS;
  }else if(serVal>Sser.maxS){
    serVal=Sser.maxS;
    }
  if(printSerVals){
    Serial.print(Sser.fName+":"+String(serVal)+" ");}
  if(printPotVals){
    Serial.print("p:"+String(potVal)+" ");}
  return serVal;
}
// Takes the input from the analog pin and sets it to the digital output to the servo. Uses maping to ajust the value. 
void analogSet(){
  ser1.write(mapPot(Sser1));
  ser2.write(mapPot(Sser2));
  ser3.write(mapPot(Sser3));
  ser4.write(mapPot(Sser4));
  ser5.write(mapPot(Sser5));

  ser6.write(mapPot(Sser6));
  Serial.println();
}
//moves all the servos sequentialy
void sequentialMove(int wtime){
  for (int cpos = 0; cpos <= 180; cpos += 20) { // goes from 0 degrees to 180 degrees
    String Scpos = String(cpos);
    serialServoReport(Scpos);
    allservosToPos(cpos, wtime);
  }
  for (int cpos = 180; cpos >= 0; cpos -= 20) { // goes from 180 degrees to 0 degrees
    String Scpos = String(cpos);
    serialServoReport(Scpos);
    allservosToPos(cpos, wtime);
  }
}


void allservosToPos(int pos, int dtime){
  ser1.write(pos);
  delay(dtime);   
  ser2.write(pos);
  delay(dtime);   
  ser3.write(pos);
  delay(dtime);   
  ser4.write(pos);
  delay(dtime);   
  ser5.write(pos);
  delay(dtime);   
  ser6.write(pos);              // tell servo to go to position in variable 'pos'
  delay(dtime);                       // waits 15ms for the servo to reach the position
}

void serialServoReport(String serSPos){
  String text1 = "Servos Pos is: ";
  String linb =  "\n";
   
  String serOutAr = text1+serSPos+linb ;
  Serial.print(serOutAr);
}

void reportServos(){
  Serial.println("Pot Vals 1: "+String(analogRead(Sser1.potNum))+" 2: "+String(analogRead(Sser2.potNum))+" 3: "+String(analogRead(Sser3.potNum))+" 4: "+String(analogRead(Sser4.potNum))+" 5: "+String(analogRead(Sser5.potNum))+" 6: "+String(analogRead(Sser6.potNum)));
  Serial.println("Servo Pos 1: "+String(mapPot(Sser1))+" 2: "+String(mapPot(Sser2))+" 3: "+String(mapPot(Sser3))+" 4: "+String(mapPot(Sser4))+" 5: "+String(mapPot(Sser5))+" 6: "+String(mapPot(Sser6)));
  }
void setup() {
  if(printSerVals||printPotVals){
    Serial.begin(9600);
    Serial.println("serial ready");
  }
  if(printSettings){
    
  }
  servosAttach();
  delay(3000);
}



void loop() {
  analogSet();

  delay(100);
  
  switch (mode) {
    //AnalogSet
    case 1:
    break;
    
    //Sequential Move
    case 2:
    sequentialMove(delayTime);
    break;
    
    //
    case 3:
    break;
    
    //
    case 4:
    String Scpos = String(constPos);
    serialServoReport(Scpos);
    allservosToPos(constPos, delayTime);
    break;

    //
    case 5:
    break;

    //
    case 6:
    Serial.println("in loop sequential start");
    sequentialMove(delayTime);
    Serial.println("in loop sequential");
    break;
    
    default:
    Serial.println("Wrong mode");
    delay(500);
    break;
  }
}