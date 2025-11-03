#include <Servo.h>

// Arduino pin assignment
#define PIN_SERVO 10

// configurable parameters
#define _DUTY_MIN 1000 // servo full clock-wise position (0 degree)
#define _DUTY_NEU 1500 // servo neutral position (90 degree)
#define _DUTY_MAX 2000 // servo full counter-clockwise position (180 degree)

#define _POS_START (_DUTY_MIN + 100)    // servo start position
#define _POS_END   (_DUTY_MAX - 100)    // servo end position

#define _SERVO_SPEED 3 // servo speed limit (unit: degree/second)
#define INTERVAL 20     // servo update interval (unit: msec)

// global variables
unsigned long last_sampling_time; // unit: msec

Servo myservo;

float duty_change_per_interval; // maximum duty difference per interval
float duty_target;    // Target duty time
float duty_curr;      // Current duty time

int toggle_interval, toggle_interval_cnt;

void setup() {
  // initialize GPIO pins
  myservo.attach(PIN_SERVO); 
  
  duty_target = duty_curr = _POS_START;
  myservo.writeMicroseconds(duty_curr);
  
  // initialize serial port
  Serial.begin(57600);

  // convert angular speed into duty change per interval.
  // Next two lines are WRONG. you have to modify.
  // duty_change_per_interval = 
  // (int)((_DUTY_MAX - _DUTY_MIN) * ((float)_SERVO_SPEED / 180.0) * ((float)INTERVAL / 1000.0));
  duty_change_per_interval = (_DUTY_MAX - _DUTY_MIN) * (_SERVO_SPEED / 180.0) * (INTERVAL / 1000.0);
// = 1000 * (3 / 180) * (0.02) = 0.3333


  // remove 'while(1) { }' lines after modify 
  Serial.print("duty_change_per_interval:");
  Serial.println(duty_change_per_interval);
  

  // initialize variables for servo update.
  toggle_interval = (180.0 / _SERVO_SPEED) * 1000 / INTERVAL;
  toggle_interval_cnt = toggle_interval;
  
  // initialize last sampling time
  last_sampling_time = 0;
}

void loop() {
  // wait until next sampling time. 
  if (millis() < (last_sampling_time + INTERVAL))
    return;

  // adjust duty_curr toward duty_target by duty_change_per_interval
  if (duty_target > duty_curr) {
    duty_curr += duty_change_per_interval;
    if (duty_curr > duty_target)
        duty_curr = duty_target;
  } else {
    duty_curr -= duty_change_per_interval;
    if (duty_curr < duty_target)
      duty_curr = duty_target;
  }

  // update servo position
  myservo.writeMicroseconds(duty_curr);

  // output the read value to the serial port
  Serial.print("Min:1000");
  Serial.print(",duty_target:"); Serial.print(duty_target);
  Serial.print(",duty_curr:");   Serial.print(duty_curr);
  Serial.println(",Max:2000");

  // toggle duty_target between _DUTY_MIN and _DUTY_MAX.
  if (toggle_interval_cnt >= toggle_interval) {
    toggle_interval_cnt = 0;
    if (duty_target == _POS_START)
      duty_target = _POS_END;
    else
      duty_target = _POS_START;
  } else {
    toggle_interval_cnt++;
  }

  // update last sampling time
  last_sampling_time += INTERVAL;
}
