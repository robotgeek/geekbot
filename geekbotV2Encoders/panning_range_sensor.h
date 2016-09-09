#ifndef PANNING_RANGE_SENSOR_H
#define PANNING_RANGE_SENSOR_H

const int SENSOR_PAN_PIN = 6;
const int DISTANCE_SENSOR_PIN = 5;
const int DISTANCE_TURN_GAIN = 9;

const int IRMODEL = 1080;

const int IR_PAN_DELAY = 1000; //milliseconds to wait for position change to occur

const int IR_PAN_CENTER = 1500;
const int IR_PAN_LEFT = 2300;
const int IR_PAN_RIGHT = 650;

unsigned long _last_distance_timestamp = millis();

int irsensorValue;

SharpIR sharp(DISTANCE_SENSOR_PIN, 25, 93, IRMODEL );
Servo servoSensor;

int getCurrentDistance()
{
  int newReading = 0;
  while ( newReading <= 0 ) //sample until valid distance is returned
  {
    newReading = sharp.distance();
    delay(10);
  }
  return newReading;
}
void processDistanceSensor()
{
  if ( _last_distance_timestamp + 50ul < millis() )
  {
    _last_distance_timestamp = millis();
    irsensorValue = irsensorValue * 0.5  + getCurrentDistance() * 0.5;
  }  
}
void lookCustom( int microseconds )
{
  servoSensor.writeMicroseconds( microseconds );
  delay(IR_PAN_DELAY); //delay so we have time to look before moving on
  irsensorValue = getCurrentDistance(); //Update current IR value manually since it may have changed drastically
}
void lookForward()
{
  servoSensor.writeMicroseconds( IR_PAN_CENTER );
  delay(IR_PAN_DELAY); //delay so we have time to look before sampling sensor
  irsensorValue = getCurrentDistance(); //Update current IR value manually since it may have changed drastically
}
void lookLeft()
{
  servoSensor.writeMicroseconds( IR_PAN_LEFT );
  delay(IR_PAN_DELAY); //delay so we have time to look before sampling sensor
  irsensorValue = getCurrentDistance(); //Update current IR value manually since it may have changed drastically
}
void lookRight()
{
  servoSensor.writeMicroseconds( IR_PAN_RIGHT );
  delay(IR_PAN_DELAY); //delay so we have time to look before sampling sensor
  irsensorValue = getCurrentDistance(); //Update current IR value manually since it may have changed drastically
}

#endif //--PANNING_RANGE_SENSOR_H
