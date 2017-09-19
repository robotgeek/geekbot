/*******************************************************************************
 *                         RobotGeek Geekbot LetsRobot.tv
 *  __________________
 *   |              |
 *   |     ___      |
 *  _|____/___\ ____|_
 *   \_/  \___/   \_/
 *
 *
 *  Wiring
 *
 *    Rotation Knob - Analog Pin 0
 *    Left Servo - Digital Pin 10
 *    Right Servo - Digital Pin 11
 *    Neck Servo - Digital Pin 9
 *    Spotlight LED - Digital Pin 12
 *    IR LED - Digital Pin 4
 *
 *    Jumper for pins 9/10/11 should be set to 'VIN'
 *    Jumper for pins 3/5/6 should be set to 'VIN'
 *
 *  Control Behavior:
 *    
 *    Move the geekbot base and controls it's accessories via serial commands.
 * 
 *******************************************************************************/

//Includes
#include <Servo.h>

//Pin Constants
const int TRIM_KNOB_PIN = 0;
const int LEFT_SERVO_PIN = 10;
const int RIGHT_SERVO_PIN = 11;
const int HEAD_TILT_PIN = 9;
const int SPOTLIGHT_PIN = 12;

const int IR_BLAST_PIN = 4;

//1 = 960 up, 360 down
//0 = 360 down, 960 up
/*
up 1011 0111 1101 1110
down 1011 0111 1011 1110
left  1011 0111 0111 1110
right 1011 0110 1111 1110
select  1011 0111 1111 0110
start 1011 0111 1110 1110

BU  1011 0111 1111 1100
BD  1011 0011 1111 1010
BL  1011 0111 1111 1010
BR  1011 0101 1111 1100
 */
uint16_t ir_up = 0b1011011111011110;
uint16_t ir_down = 0b1011011110111110;
uint16_t ir_left = 0b1011011101111110;
uint16_t ir_right = 0b1011011011111110;
uint16_t ir_select = 0b1011011111110110;
uint16_t ir_start = 0b1011011111101110;
uint16_t ir_btn_up = 0b1011011111111100;
uint16_t ir_btn_down = 0b1011001111111010;
uint16_t ir_btn_left = 0b1011011111111010;
uint16_t ir_btn_right = 0b1011010111111100;

const int micdelay=10;
void blastIR( uint16_t value )
{
  unsigned long microsnow;
  bool pin_state = false;
  digitalWrite(IR_BLAST_PIN, LOW);
  for ( int pulse_index = 15; pulse_index >= 0; --pulse_index )
  {
    if ( (value >> pulse_index) & 1 == 1 )
    {
      //Serial.println( "HIGH" );
      microsnow = micros();
      while( microsnow + 960 > micros() )
      {
        digitalWrite(IR_BLAST_PIN, HIGH);
        delayMicroseconds(micdelay);
        digitalWrite(IR_BLAST_PIN, LOW);
        delayMicroseconds(micdelay);
      }
      
      digitalWrite(IR_BLAST_PIN, LOW);
      delayMicroseconds( 360 );
    }
    else
    {
      //Serial.println( "LOW" );

      microsnow = micros();
      while( microsnow + 360 > micros() )
      {
        digitalWrite(IR_BLAST_PIN, HIGH);
        delayMicroseconds(micdelay);
        digitalWrite(IR_BLAST_PIN, LOW);
        delayMicroseconds(micdelay);
      }

      digitalWrite(IR_BLAST_PIN, HIGH);
      delayMicroseconds( 960 );
    }
  }
  
  digitalWrite(IR_BLAST_PIN, LOW);
  delay(50); //50ms delay between signals
}

//Speed constants for servo wheel speeds in microseconds
const int CCWMaxSpeed = 1750; //Left wheel forward speed
const int CCWMedSpeed = 1610;
const int CCWMinSpeed = 1580;
const int CWMaxSpeed = 1250; //Right wheel forward speed
const int CWMedSpeed = 1350;
const int CWMinSpeed = 1400;
const int ServoStopSpeed = 1500; //Servo pulse in microseconds for stopped servo
const int ServoRotateSpeedHigh = 50; //For in place rotation. Applied to CW and CCW_MIN_SPEEDs
const int ServoRotateSpeedLow = 25; //For in place rotation. Applied to CW and CCW_MIN_SPEEDs
const int MaxTrimAdjustment = 100; //Limit rotation knob trim effect in microseconds

const unsigned long controlTimeout = 250; //Milliseconds to wait without control input for stopping robot
unsigned long commandTimestamp = millis(); //Milliseconds since last command was received
int MoveFrontBack; //State var for base control
int MoveLeftRight; //State var for base control

//Servo control
enum SpeedSelections
{
  SPEED_MIN,
  SPEED_MED,
  SPEED_MAX
};
Servo servoNeck;
Servo servoLeft, servoRight; //Wheel servo objects
int servoRotateSpeed = ServoRotateSpeedHigh; //For rotating in place
int servoDriveTurningSpeed = 40; //For turning while driving
int servoSpeedLeft = ServoStopSpeed; //Left servo speed to be sent to servo
int servoSpeedRight = ServoStopSpeed; //Right servo speed to be sent to servo
int leftFwdSpeed = CCWMaxSpeed; //Currently selected left forward speed
int leftRevSpeed = CWMaxSpeed; //Currently selected left reverse speed
int rightFwdSpeed = CWMaxSpeed; //Currently selected right forward speed
int rightRevSpeed = CCWMaxSpeed; //Currently selected right reverse speed

const int neckUp = 2000;
const int neckDown = 1000;
int neckPosition = 1500;

//Wheel speed trim control
int wheelSpeedTrim = 0; //Current wheel speed trim from rotation knob
void updateDriveTrim()
{
  int knob_value = analogRead( TRIM_KNOB_PIN );
  wheelSpeedTrim = map( knob_value, 0, 1023, -MaxTrimAdjustment, MaxTrimAdjustment );
}

//Incoming data handling
int inputbufferindex = 0;
char inputbuffer[ 100 ] = { 0 };

void ParseIncoming( char* incoming )
{
  int checklen = 0;

  int argval = -1;
  const char* ptr = strchr( incoming, ' ' );

  if( ptr )
  {
    checklen = ( ptr + 1 ) - incoming;
    argval = atoi( ptr );
  }
  else
  {
    checklen = strlen( incoming );
  }

  if( strncmp( incoming, "lookup", 6 ) == 0 )
  {
    if ( neckPosition + 100 <= neckUp )
    {
      neckPosition += 100;
    }
    servoNeck.writeMicroseconds( neckPosition );
  }
  else if( strncmp( incoming, "lookdown", 8 ) == 0 )
  {
    if ( neckPosition - 100 >= neckDown )
    {
      neckPosition -= 100;
    }
    servoNeck.writeMicroseconds( neckPosition );
  }
  else if( strncmp( incoming, "lighton", 7 ) == 0 )
  {
    digitalWrite( SPOTLIGHT_PIN, HIGH );
  }
  else if( strncmp( incoming, "lightoff", 8 ) == 0 )
  {
    digitalWrite( SPOTLIGHT_PIN, LOW );
  }
  else if( strncmp( incoming, "stop", 4 ) == 0 )
  {
    commandTimestamp = 0;
  }
  else if( strncmp( incoming, "f", 1 ) == 0 )
  {
    commandTimestamp = millis();
    MoveFrontBack = 1;
    MoveLeftRight = 0;
  }
  else if( strncmp( incoming, "b", 1 ) == 0 )
  {
    commandTimestamp = millis();
    MoveFrontBack = -1;
    MoveLeftRight = 0;
  }
  else if( strncmp( incoming, "l", 1 ) == 0 )
  {
    commandTimestamp = millis();
    MoveLeftRight = -1;
    MoveFrontBack = 0;
  }
  else if( strncmp( incoming, "r", 1 ) == 0 )
  {
    commandTimestamp = millis();
    MoveLeftRight = 1;
    MoveFrontBack = 0;
  }
  else if( strncmp( incoming, "irfwd", checklen ) == 0 )
  {
    blastIR( ir_up );
  }
  else if( strncmp( incoming, "irback", checklen ) == 0 )
  {
    blastIR( ir_down );
  }
  else if( strncmp( incoming, "irleft", checklen ) == 0 )
  {
    blastIR( ir_left );
  }
  else if( strncmp( incoming, "irright", checklen ) == 0 )
  {
    blastIR( ir_right );
  }
  else if( strncmp( incoming, "irstart", checklen ) == 0 )
  {
    blastIR( ir_start );
  }
  else if( strncmp( incoming, "irselect", checklen ) == 0 )
  {
    blastIR( ir_select );
  }
  else if( strncmp( incoming, "irbtn0", checklen ) == 0 )
  {
    blastIR( ir_btn_up );
  }
  else if( strncmp( incoming, "irbtn1", checklen ) == 0 )
  {
    blastIR( ir_btn_down );
  }
  else if( strncmp( incoming, "irbtn2", checklen ) == 0 )
  {
    blastIR( ir_btn_left );
  }
  else if( strncmp( incoming, "irbtn3", checklen ) == 0 )
  {
    blastIR( ir_btn_right );
  }
  else
  {
  }
}

void setup()
{
  Serial.begin( 9600 );

  pinMode(IR_BLAST_PIN, OUTPUT);
  digitalWrite( IR_BLAST_PIN, LOW );
  pinMode( SPOTLIGHT_PIN, OUTPUT );
  digitalWrite( SPOTLIGHT_PIN, LOW );

  servoLeft.attach( LEFT_SERVO_PIN );
  servoRight.attach( RIGHT_SERVO_PIN );
  servoLeft.writeMicroseconds( servoSpeedLeft );
  servoRight.writeMicroseconds( servoSpeedRight );

  servoNeck.attach( HEAD_TILT_PIN );
  servoNeck.writeMicroseconds( neckPosition );
}

void loop()
{
  while( Serial.available() )
  {
    char input = Serial.read();
    if( input == 0x0d || input == 0x0a )
    {
      if( inputbufferindex == 0 )
        break;

      inputbuffer[ inputbufferindex++ ] = 0;
      ParseIncoming( inputbuffer );
      inputbuffer[ 0 ] = 0;
      inputbufferindex = 0;
    }
    else
    {
      inputbuffer[ inputbufferindex++ ] = input;
    }

    if( inputbufferindex == sizeof(inputbuffer) )
    {
      inputbufferindex = 0;
      inputbuffer[ inputbufferindex ] = 0;
    }
  }

  updateDriveTrim();
  if(MoveFrontBack>0)
  {
    servoSpeedLeft = leftFwdSpeed + wheelSpeedTrim;
    servoSpeedRight = rightFwdSpeed + wheelSpeedTrim;
    if ( MoveLeftRight<0 )
    {
      servoSpeedLeft -= servoDriveTurningSpeed;
    }
    if ( MoveLeftRight>0 )
    {
      servoSpeedRight += servoDriveTurningSpeed;
    }
  }
  else if(MoveFrontBack<0)
  {
    servoSpeedLeft = leftRevSpeed;
    servoSpeedRight = rightRevSpeed;
    if ( MoveLeftRight<0 )
    {
      servoSpeedLeft += servoRotateSpeed;
    }
    if ( MoveLeftRight>0 )
    {
      servoSpeedRight -= servoRotateSpeed;
    }
  }
  else if (MoveLeftRight<0 )
  {
    servoSpeedLeft = CWMinSpeed - servoRotateSpeed;
    servoSpeedRight = CWMinSpeed - servoRotateSpeed;
  }
  else if ( MoveLeftRight>0 )
  {
    servoSpeedLeft = CCWMinSpeed + servoRotateSpeed;
    servoSpeedRight = CCWMinSpeed + servoRotateSpeed;
  }
  
  if ( commandTimestamp + controlTimeout < millis() )
  {
    servoSpeedLeft = ServoStopSpeed;
    servoSpeedRight = ServoStopSpeed;
    MoveLeftRight=0;
    MoveFrontBack=0;
  }

  servoLeft.writeMicroseconds(servoSpeedLeft);
  servoRight.writeMicroseconds(servoSpeedRight);
}
