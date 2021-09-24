#include <Arduino.h>
#include <Encoder.h>
#include <Joystick.h>
#include <Servo.h>

#define PIN_ROT_A 2
#define PIN_ROT_B 3
#define PIN_ROT_RESET 4

#define PIN_TRIM_SERVO 16


Servo indicator;
int indicatorPosition = 90;

Encoder rotary(PIN_ROT_A,3);
long rotaryPosition = 0;

int16_t TRIM_MIN = (int16_t) -512;
int16_t TRIM_MAX = (int16_t) 512;

long SERVO_MIN = 60;
long SERVO_MAX = 120;

Joystick_ Joystick(
  JOYSTICK_DEFAULT_REPORT_ID, 
  JOYSTICK_TYPE_JOYSTICK,
  0,      // # buttons
  0,      // # hat switches
  true,   // X-axis
  false,  // Y
  false,  // Z
  false,  // Rx
  false,  // Ry
  false,  // Rz
  false,  // Rudder
  false,  // Throttle
  false,  // Accellerator
  false,  // Brake
  false   // Steering
);

int lastButtonState = 0;

void setup() {  
  pinMode(PIN_ROT_RESET, INPUT_PULLUP);

  indicator.attach(PIN_TRIM_SERVO);

  Joystick.begin();
  Joystick.setXAxisRange(TRIM_MIN, TRIM_MAX);
  // Joystick.setRxAxisRange(TRIM_MIN, TRIM_MAX);
}

void loop() {
  // Read rotary position and update joystick if changed
  int16_t newPosition = (int16_t) rotary.read();

  // Clamp rotary value
  if (newPosition < TRIM_MIN) {
    rotary.write(TRIM_MIN);
    newPosition = TRIM_MIN;
  } else if (newPosition > TRIM_MAX) {
    rotary.write(TRIM_MAX);
    newPosition = TRIM_MAX;
  }

  // If changed, update joystick
  if (newPosition != rotaryPosition) {
    rotaryPosition = newPosition;
    Joystick.setXAxis(newPosition); 
    // Joystick.setRxAxis(newPosition);

    // Update indicator if it has changed
    int newIndicatorPosition = (int) map(newPosition, TRIM_MIN, TRIM_MAX, SERVO_MIN, SERVO_MAX);
    if (indicatorPosition != newIndicatorPosition) {
      indicator.write(newIndicatorPosition);
      indicatorPosition = newIndicatorPosition;
    }
  }

  // Set to zero if button pressed
	int currentButtonState = !digitalRead(pinToButtonMap);
	if (currentButtonState != lastButtonState)
	{
    lastButtonState = currentButtonState;
    rotary.write(0);
	}
}

