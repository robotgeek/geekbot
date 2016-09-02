/******************************\
 * IR sensor on panning servo *
\******************************/
#define DISTANCE_TURN_GAIN 9 //TODO: was high at 10 //8 was kinda low

const int SENSOR_PAN_PIN = 6;
const int DISTANCE_SENSOR_PIN = 5;
const int IRMODEL = 1080;
SharpIR sharp(DISTANCE_SENSOR_PIN, 25, 93, IRMODEL );
int irsensorValue;
const int IR_PAN_DELAY = 1000; //milliseconds to wait for position change to occur
const int IR_PAN_CENTER = 1500;
const int IR_PAN_LEFT = 2300; //2300;
const int IR_PAN_RIGHT = 650; //544;
Servo servoSensor;
unsigned long last_distance_timestamp = millis();

int getCurrentDistance()
{
  int newReading = 0;
  while ( newReading <= 0 ) //sample until valid distance is returned
  {
    newReading = sharp.distance();
    delay(5);
  }
  return newReading;
}
void processDistanceSensor()
{
  if ( last_distance_timestamp + 100ul < millis() )
  {
    last_distance_timestamp = millis();
    irsensorValue = getCurrentDistance();
  }  
}

void lookLeft()
{
  servoSensor.writeMicroseconds( IR_PAN_LEFT );
  delay(IR_PAN_DELAY); //delay so we have time to look before sampling sensor
}
void lookRight()
{
  servoSensor.writeMicroseconds( IR_PAN_RIGHT );
  delay(IR_PAN_DELAY); //delay so we have time to look before sampling sensor
}
