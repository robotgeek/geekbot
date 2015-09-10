#include <Servo.h> 

#define PAN_SERVO_PIN 5
#define WRIST_PIN 6
#define GRIPPER_PIN 9
#define LEFT_DRIVE_PIN 10
#define RIGHT_DRIVE_PIN 11

#define FSR_LEFT 6
#define FSR_RIGHT 7
#define KNOB_PIN 2

Servo gripperServo, wristServo, cameraServo, leftDriveServo, rightDriveServo, servoPan ;

int fsrLeftVal, fsrRightVal;

int gripperVal = 180;
int wristVal = 90;


long previousMillis = 0;        // will store last time LED was updated

// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long interval = 10;           // interval at which to blink (milliseconds)

int stableCount;
int closeCount;

void setup()
{
  
  servoPan.attach(PAN_SERVO_PIN);
  gripperServo.attach(GRIPPER_PIN, 750, 2400);
  wristServo.attach(WRIST_PIN);
  //cameraServo.attach(CAMERA_PIN);
  leftDriveServo.attach(LEFT_DRIVE_PIN);
  rightDriveServo.attach(RIGHT_DRIVE_PIN);
  
  servoPan.write(1500); //write 1500us, 90 degrees
  wristServo.write(1500); //write 1500us, 90 degrees
  gripperServo.write(1500); //write 1500us, 90 degrees
  leftDriveServo.write(1500); //write 1500us, 90 degrees
  rightDriveServo.write(1500); //write 1500us, 90 degrees
  
  gripperServo.write(gripperVal);
  wristServo.write(wristVal);
  
  Serial.begin(38400);
  
 // pinMode(15, INPUT_PULLUP);
  
  delay(1000);
  
}


void loop()
{
  fsrLeftVal = analogRead(FSR_LEFT);
  delay(2);
  fsrRightVal = analogRead(FSR_RIGHT);
  delay(2);
  
  
  if(fsrLeftVal < 400 || fsrRightVal < 400)
  {
    gripperVal = gripperVal - 1;
    delay(10);
  }
   if (fsrLeftVal > 800 || fsrRightVal > 800)
  {
    gripperVal = gripperVal + 1;
    delay(10);
  }
  
  if((fsrLeftVal <= 800 && fsrLeftVal >= 400)||(fsrRightVal <= 800 && fsrRightVal >= 400))
  {
    stableCount = stableCount + 1;
  }
  
  if(gripperVal < 5)
  {
    Serial.println("checklcc");
    if(fsrLeftVal < 80)
    {
      closeCount++;
    }
  }
  else 
  {
    closeCount = 0;
  }
  
  if (closeCount > 20)
  {
    
    gripperVal = 180;
  }
  
  
  if (stableCount > 50)
  {
    
    wristServo.write(125);
    cameraServo.write(135);
    leftDriveServo.write(180);
    rightDriveServo.write(180);
    delay(500);
    leftDriveServo.write(180);
    rightDriveServo.write(0);
    delay(5000);
    leftDriveServo.write(0);
    rightDriveServo.write(0);
    delay(500);
    
    leftDriveServo.write(90);
    rightDriveServo.write(90);
    wristServo.write(90);
    cameraServo.write(90);
    //gripperServo.write(180);
    gripperVal = 180;
    stableCount = -200;
  }
  
  
  
  gripperVal = min(180, gripperVal);
  gripperVal = max(0, gripperVal);
  
  //gripperVal = map(analogRead(KNOB_PIN), 0, 1023, 0, 180);
 // gripperVal = gripperVal;
  
   
    
    gripperServo.write(gripperVal);
    delay(10);
  
  
   unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;  
   // delay(10);
//    
//    Serial.print(gripperVal);
//    Serial.print(" ");
//    Serial.print(fsrLeftVal);
//    
//    if(fsrLeftVal < 10)
//    {
//    Serial.print("   ");
//    }
//    
//    else if(fsrLeftVal < 100)
//    {
//    Serial.print("  ");
//    }
//    else if(fsrLeftVal < 1000)
//    {
//    Serial.print(" ");
//    }
//    
//    Serial.print(" ");
//    Serial.print(fsrRightVal);
//    Serial.println(" ");
//  

//    
    Serial.print(fsrLeftVal);
    Serial.print(" ");
    Serial.print(gripperVal);
    Serial.print(" ");
    Serial.println(closeCount);

  }
  
  
  
  
  
}
