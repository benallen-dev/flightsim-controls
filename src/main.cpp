#include <Arduino.h>
#include <Encoder.h>
#include <Joystick.h>
#include <Servo.h>
#include <Button.h>

#define PIN_ROT_A 2
#define PIN_ROT_B 3
#define PIN_ROT_RESET 4
#define PIN_THROTTLE A0
#define PIN_MIXTURE A1

#define PIN_TRIM_SERVO 16
#define PIN_BRAKE_UP 6
#define PIN_BRAKE_DN 7
#define PIN_FLAPS_UP 8
#define PIN_FLAPS_DN 9

// Note: The version of the Button library you can get from the platformio
// registry is out of date and doesn't include a parameter for debounce time.
// The default is a rather silly 500ms, so you need to modify the debounce
// time to something more sensible in the libdeps folder. I used 50ms.
Button buttonBrakeUp(PIN_BRAKE_UP);
Button buttonBrakeDn(PIN_BRAKE_DN);
Button buttonFlapsUp(PIN_FLAPS_UP);
Button buttonFlapsDn(PIN_FLAPS_DN);
Button buttonTrimReset(PIN_ROT_RESET);

Servo indicator;
int indicatorPosition = 90;

Encoder rotary(PIN_ROT_A, PIN_ROT_B);
long rotaryPosition = 0;

int16_t TRIM_MIN = (int16_t) -512;
int16_t TRIM_MAX = (int16_t) 512;

long SERVO_MIN = 60;
long SERVO_MAX = 120;

Joystick_ Joystick(
  JOYSTICK_DEFAULT_REPORT_ID, 
  JOYSTICK_TYPE_JOYSTICK,
  4,      // # buttons
  0,      // # hat switches
  true,   // X-axis           (trim)
  true,   // Y                (throttle)
  true,   // Z                (mixture)
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
  buttonBrakeDn.begin();
  buttonBrakeUp.begin();
  buttonFlapsDn.begin();
  buttonFlapsUp.begin();
  buttonTrimReset.begin();

  indicator.attach(PIN_TRIM_SERVO);

  Joystick.begin();
  Joystick.setXAxisRange(TRIM_MIN, TRIM_MAX);
  Joystick.setYAxisRange(0, 1024); // not configurable because the ADC only has 10-bit resolution
  Joystick.setZAxisRange(0, 1024); // not configurable because the ADC only has 10-bit resolution
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

    // Update indicator if it has changed
    int newIndicatorPosition = (int) map(newPosition, TRIM_MIN, TRIM_MAX, SERVO_MIN, SERVO_MAX);
    if (indicatorPosition != newIndicatorPosition) {
      indicator.write(newIndicatorPosition);
      indicatorPosition = newIndicatorPosition;
    }
  }

  // Buttons
  if (buttonTrimReset.pressed()) rotary.write(0);

  if (buttonFlapsUp.toggled()) {
    Joystick.setButton(0, buttonFlapsUp.read() == Button::PRESSED);
  }
  if (buttonFlapsDn.toggled()) {
    Joystick.setButton(1, buttonFlapsDn.read() == Button::PRESSED);
  }
  if (buttonBrakeUp.toggled()) {
    Joystick.setButton(2, buttonBrakeUp.read() == Button::PRESSED);
  }
  if (buttonBrakeDn.toggled()) {
    Joystick.setButton(3, buttonBrakeDn.read() == Button::PRESSED);
  }

  /* THROTTLE AND MIXTURE */
  int throttleRead = analogRead(PIN_THROTTLE);
  int mixtureRead = analogRead(PIN_MIXTURE);

  Joystick.setYAxis(1024 - throttleRead);
  Joystick.setZAxis(1024 - mixtureRead);
}
