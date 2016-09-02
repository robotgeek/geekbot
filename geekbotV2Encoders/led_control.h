#define LED_LEFT 7
#define LED_RIGHT 4
unsigned long last_led_blink = millis();
unsigned long led_flash_delay = 100; //time in milliseconds for LED flashing
int led_blinks_left = 0;
int led_blinks_right = 0;
bool led_state_left = false;
bool led_state_right = false;

void flashLEDs( int numTimes, int flashDelay )
{
  while ( --numTimes >= 0 )
  {
    digitalWrite( LED_LEFT, HIGH );
    digitalWrite( LED_RIGHT, HIGH );
    delay(flashDelay);
    digitalWrite( LED_LEFT, LOW );
    digitalWrite( LED_RIGHT, LOW );
    delay(flashDelay);
  }
}
void setBlinkSpeed( unsigned long milliseconds )
{
  led_flash_delay = milliseconds;
}
void setBlinksLeft( int numBlinks )
{
  if ( led_blinks_left == 0 )
  {
    led_blinks_left = numBlinks;
  }
}
void setBlinksRight( int numBlinks )
{
  if ( led_blinks_right == 0 )
  {
    led_blinks_right = numBlinks;
  }
}
void processLEDs()
{
  if ( last_led_blink + led_flash_delay < millis() )
  {
    last_led_blink = millis();
    if ( led_blinks_left > 0 )
    {
      led_state_left = !led_state_left;
      --led_blinks_left;
      digitalWrite( LED_LEFT, led_state_left );
    }
    if ( led_blinks_right > 0 )
    {
      led_state_right = !led_state_right;
      --led_blinks_right;
      digitalWrite( LED_RIGHT, led_state_right );
    }
  }
}
