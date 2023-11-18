/*
 * Arduino Camera platform driver
 * 
 * CAUTION：Do not choose PinChangeInterrupts.
 * That makes conflict withSoftwareserial
 *    
 *   by Tsukasa Takao@Umineco ltd. 2023/11
 */
#include <SoftwareSerial.h>

#define DEBUG

// Depth senser
#define RX_PIN 9
#define TX_PIN 10
#define BUFFLEN 50
int Distance;
int Depth = 300; // mm 
int minDepth = 500; // Don't work under 50cm
boolean Dvalid = false; // If true, the distance data is reliable.

// Ball sensor
#define BallSensePin 11 // Motor senser pin
#define BOUNCETIME 1000 // neglect ball sensor boucing 
boolean BallSenseFlag = HIGH;
unsigned long previousMillis = 0;

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

SoftwareSerial dSerial(RX_PIN, TX_PIN);

String Cmd; // コマンド
#define START "start"
#define REWIND "rewind"

// 1回転
void rotate(int count){
  for (int i = 0; i < stepsPerRev*count; i++) {
      digitalWrite(stepXPin, HIGH);
      digitalWrite(stepYPin, HIGH);
      digitalWrite(stepZPin, HIGH);
      delay(pulseWidth);
      digitalWrite(stepXPin, LOW);
      digitalWrite(stepYPin, LOW);
      digitalWrite(stepZPin, LOW);
      delay(pulseWidth);    
  }
  #ifdef DEBUG
    Serial.print("*");
  #endif
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
           // Serial.print("State:");
           // Serial.print(state);
           // Serial.print("    Distance:");
           // Serial.print(Distance);
           // Serial.print(" mm");
           // Serial.print("\r\n");
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


void getCmd(){
  // command process
  if (Serial.available() > 0){
    Cmd  = Serial.readString();
    Cmd.trim(); // remove blank, CR, LF

    int placeholder = Cmd.indexOf("=");
    
    if (placeholder != -1){
       // Depth command process
      String key = Cmd.substring(0,placeholder);
      String value = Cmd.substring(placeholder+1);
      if ( key == "depth" ){
        Depth = value.toInt();

        Serial.println( "Depth changed to " +  value);
      } else {
        Serial.println("Unknown command: "+ key);
      }
    }
  }
}

boolean CheckBall(){
  // Wire Sensor chack. Usually HIGH, if metal ball touched, LOW.
  if ( (millis() - BOUNCETIME) > previousMillis ){
    // valid change
    BallSenseFlag = digitalRead(BallSensePin);
  }
  previousMillis = millis();
} 

void StartProcess(){
  digitalWrite(enPin, LOW);
  if  (minDepth < Distance) {
    // go down
    // set direction
    moveDown();
    while (1){
      rotate(2);
      getDistance();
      
      if ( (Depth > Distance) & Dvalid){
        digitalWrite(enPin, HIGH); // stop motor
        Serial.println("END RC=0");
        #ifdef DEBUG
          Serial.println("Distance Sensed");
        #endif
        
        break;
      }
      
      CheckBall();
      
      if ( BallSenseFlag == LOW ){ // sense low means detected ball.
        digitalWrite(enPin, HIGH);
        Serial.println("END RC=1");
 
        #ifdef DEBUG
          Serial.println("Ball Sensed");
        #endif
        
        break;
      }
    } // end of while
  } // end of minDepth
  digitalWrite(enPin, HIGH);
  Cmd = "";
}

void RewindProcess(){
  moveUp();

  while (1){
    rotate(1);

    CheckBall();
    if (BallSenseFlag == LOW ){
      Serial.println("END RC=0");
      break;    
    }
  }
  Cmd = "";
}

void setup() {

  // Sensor setup
  pinMode(BallSensePin, INPUT_PULLUP);
  
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
  // dSerial.begin(19200);
  // dSerial.begin(38400);
  // dSerial.begin(57600);
  dSerial.begin(115200);
  dSerial.flush();

  Serial.println("Serial/SoftwareSerial Ready.");
}

void loop(){
  
  getCmd();

  getDistance();
 
  // command process
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
    
} // end loop