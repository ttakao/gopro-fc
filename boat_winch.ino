/*
 * Arduino Camera platform driver
 * 
 * CAUTION : NeoSWSerial.h is modified to accept GPIO interruption.
 *      
 *   by Tsukasa Takao@Umineco ltd. 2023/11, 2024/6
 */
#include <NeoSWSerial.h>
#include <PinChangeInterrupt.h>

#define DEBUG

// Depth senser
#define RXPin 9
#define TXPin 10
#define BUFFLEN 50
int Distance; // mm
int Depth = 2000; // mm 
int minDepth = 500; // Don't work under 50cm
boolean Dvalid = false; // If true, the distance data is reliable.

// Ball sensor
#define BallSensePin 11 // Motor senser pin
volatile boolean BallSense = HIGH; // By program and Interruption Handler

// CNC motor drive board
const int stepXPin = 2; // X-axis step pin
const int stepYPin = 3; // Y-axis step pin
const int stepZPin = 4; // Z-axis step pin

const int dirXPin = 5; // X-axis direction pin
const int dirYPin = 6; // Y-axis direction pin
const int dirZPin = 7; // Z-axis direction pin

const int enPin = 8; // spindle enable pin

const int stepsPerRev = 200; // 200カウントで1回転
int pulseWidth = 3; // microsecond パルス幅　テスト結果の最小値

NeoSWSerial dSerial(RXPin, TXPin);

String Cmd; // コマンド
#define DEPTH "depth="
#define START "start"
#define STOP "stop"
#define REWIND "rewind"

// 1回転
void rotate(int count){
  for (int i = 0; i < stepsPerRev*count; i++) {
    
      if (BallSense == LOW){ // By some reason, stop rotation !
        break; 
      }
      
      digitalWrite(stepXPin, HIGH);
      digitalWrite(stepYPin, HIGH);
      digitalWrite(stepZPin, HIGH);
      delay(pulseWidth);
      digitalWrite(stepXPin, LOW);
      digitalWrite(stepYPin, LOW);
      digitalWrite(stepZPin, LOW);
      delay(pulseWidth); 

  }
}

void moveDown(){
  #ifdef DEBUG
  Serial.println("directin DOWN");
  #endif 
  digitalWrite(dirXPin, HIGH);
  digitalWrite(dirYPin, LOW);
  digitalWrite(dirZPin, LOW);
}

void moveUp(){
  #ifdef DEBUG
  Serial.println("directin UP");
  #endif 
   digitalWrite(dirXPin, LOW);
   digitalWrite(dirYPin, HIGH);
   digitalWrite(dirZPin, HIGH);
}

void getDistance(){
  char cStr[BUFFLEN];
  unsigned short usCnt = 0;
  int state;

  while (dSerial.available())
    {
      cStr[usCnt] = dSerial.read();
      usCnt=usCnt+1;

      if ((cStr[0] == 'S') && (cStr[1] == 't')&&(cStr[2] == 'a') )
      {
        if ((cStr[usCnt-2] == ' ') && (cStr[usCnt-1] == ','))
        {
          sscanf(cStr, "State;%d ,", &state);
          if (state == 0){
            Dvalid = true;
          } else {
            Dvalid = false;
          }
                     
          usCnt=0; // reset count
        }
      }
      else if ((cStr[0] == 'd') && (cStr[1] == ':'))
      {
        if ((cStr[usCnt-2] == '\r') && (cStr[usCnt-1] == '\n'))
        {
          sscanf(cStr, "d: %d\r\n", &Distance);

          #ifdef DEBUG
            Serial.print("State:");
            Serial.print(state);
            Serial.print("    Distance:");
            Serial.print(Distance);
            Serial.print(" mm");
            Serial.print("\r\n");
          #endif
          
          usCnt=0; // reset counter
        }
      }
      else if (usCnt > 2) {
        usCnt=usCnt-1;
        memcpy(&cStr[0], &cStr[1], usCnt);
      }
    }
}


void getDepth(){
  // Depth command process
  String key = Cmd.substring(0,placeholder);
  String value = Cmd.substring(placeholder+1);
  
  if ( key == DEPTH ){
    Depth = value.toInt();

    Serial.println( "Depth changed to " +  value);
  } else {
    Serial.println("Unknown command: "+ key);
  }
}

void SWSerial_ISR(){
   NeoSWSerial::rxISR( *portInputRegister( digitalPinToPort( RXPin ) ) );
}
void Ball_ISR(){
    BallSense = LOW;
}

void StartProcess(){
  // too short depth, do nothing.
  if  (minDepth > Distance) {
    return;
  }

  #ifdef DEBUG
    Serial.println("START Proecess start");
  #endif
  // go down
  moveDown();
  // before loop, reset ball sensor
  BallSense = HIGH;
  while (1){
    rotate(2);
    if (BallSense == LOW){ // if stopped by sense 
      Serial.println("END RC=1");
      break;
    }

    if (Cmd == STOP){
      Serial.println("END RC=1");
      break;
    }

    getDistance();
      
    if ( (Depth > Distance) & Dvalid){
      Serial.println("END RC=0");
      
      #ifdef DEBUG
        Serial.println("Distance Sensed");
      #endif
        
      break;
    }
  } // end of while

}

void RewindProcess(){
  #ifdef DEBUG
    Serial.println("REWIND Proecess start");
  #endif

  moveUp();
  // before loop, reset ball sense
  BallSense = HIGH;
  
  while (1){
    
    rotate(3);
    
    if (BallSense == LOW ){
      Serial.println("END RC=0");

      break;    
    }
    // minimum depth 
    getDistance();
    if ( (minDepth > Distance) & Dvalid){
      Serial.println("END RC=0");
      break;
    }
  }
}

void setup() {

  // Sensor setup
  pinMode(BallSensePin, INPUT_PULLUP); // always HIGH except detected.
  
  // Motor setup
  pinMode(stepXPin, OUTPUT);
  pinMode(stepYPin, OUTPUT);
  pinMode(stepZPin, OUTPUT);
  pinMode(dirXPin,  OUTPUT);
  pinMode(dirYPin,  OUTPUT);
  pinMode(dirZPin,  OUTPUT);
  pinMode(enPin,    OUTPUT);
  
  // Serial baud rate 9600bps
  Serial.begin(9600);
  delay(200);
  
  // dSerial.begin(9600);
  dSerial.begin(19200);
  // dSerial.begin(38400);
  dSerial.flush();

  attachPCINT(digitalPinToPCINT( RXPin) , SWSerial_ISR, CHANGE);
  attachPCINT(digitalPinToPCINT(BallSensePin), Ball_ISR, FALLING);
  
  digitalWrite(enPin, LOW); // allways enable

  Serial.println("Serial/NeoSWSerial Ready.");
}

void loop(){
    // command process
  if (Serial.available() > 0){

    Cmd  = Serial.readString();
    Cmd.trim(); // remove blank, CR, LF

    if (Cmd.indexOf("=") != -1){    
      getDepth();
    }
    #ifdef DEBUG
      Serial.println(Cmd);
    #endif
  }

  getDistance();
 
  // command process, but stop is emergency.
  if (Cmd == START){
    #ifdef DEBUG
    Serial.println("Start !");
    #endif
    StartProcess();
  } else if (Cmd == REWIND){
    #ifdef DEBUG
    Serial.println("Rewind !");
    #endif
    RewindProcess();
  }
  // done command.
  Cmd = "";

    
} // end loop