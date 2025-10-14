#include <Servo.h>

// Arduino pin assignment
#define PIN_LED   9   // LED active-low
#define PIN_TRIG  12  // sonar sensor TRIGGER
#define PIN_ECHO  13  // sonar sensor ECHO
#define PIN_SERVO 10  // servo motor

// configurable parameters for sonar
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25       // sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 180.0   // minimum distance to be measured (unit: mm)
#define _DIST_MAX 360.0   // maximum distance to be measured (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL)     // coefficient to convert duration to distance

#define _EMA_ALPHA 0.4

// Target Distance Range (in mm)
#define _TARGET_LOW  250.0
#define _TARGET_HIGH 290.0

// Servo pulse range
#define _DUTY_MIN 1000  // 0도
#define _DUTY_NEU 1500  // 90도
#define _DUTY_MAX 2000  // 180도

// global variables
float  dist_ema = _DIST_MAX;  // EMA 필터 적용 거리값
float  dist_prev = _DIST_MAX; // 이전 측정값
unsigned long last_sampling_time = 0;

Servo myservo;

// -------------------- 초기 설정 --------------------
void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  myservo.attach(PIN_SERVO);
  myservo.writeMicroseconds(_DUTY_NEU); 

  Serial.begin(57600);
}

// -------------------- 메인 루프 --------------------
void loop() {
  float dist_raw, dist_filtered;

  // wait until next sampling time.
  // millis() returns the number of milliseconds since the program started. 
  // will overflow after 50 days.
  if (millis() < last_sampling_time + INTERVAL)
    return;

  // 거리 측정
  dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

  // the range filter
  if ((dist_raw == 0.0) || (dist_raw > _DIST_MAX)) { // 최댓값보다 크거나 값이 0일때
      dist_filtered = dist_prev;
  } else if (dist_raw < _DIST_MIN) { // 최솟값보다 작을 떄
      dist_filtered = dist_prev;
  } else {    
      dist_filtered = dist_raw;
      dist_prev = dist_raw;
  }

  // ===== EMA 필터 적용 =====
  // EMA 식: EMA_new = α * 현재값 + (1-α) * 이전 EMA값
  dist_ema = _EMA_ALPHA * dist_filtered + (1.0 - _EMA_ALPHA) * dist_ema;

  // ===== 거리 기반 서보 제어 =====
  float angle;

  if (dist_ema <= _DIST_MIN) {
    angle = 0;  // 18cm 이하 → 0도

  }
  else if (dist_ema >= _DIST_MAX) {
    angle = 180;  // 36cm 이상 → 180도

  }
  else {
    // 18~36cm 사이라면 비례 계산
    angle = (dist_ema - _DIST_MIN) * (180.0 / (_DIST_MAX - _DIST_MIN));

  }

  // ===== led 제어 =====
  if (dist_raw >= _DIST_MIN && dist_raw <= _DIST_MAX) { // 180~360mm
    digitalWrite(PIN_LED, LOW); // ON 
  } else {
    digitalWrite(PIN_LED, HIGH);  // OFF
  }

  // 서보 각도 적용 (0~180도)
  myservo.write(angle);

  // output the distance to the serial port
  Serial.print("Min:");    Serial.print(_DIST_MIN);
  Serial.print(",Low:");   Serial.print(_TARGET_LOW);
  Serial.print(",dist:");  Serial.print(dist_raw);
  Serial.print(",Servo:"); Serial.print(myservo.read());  
  Serial.print(",High:");  Serial.print(_TARGET_HIGH);
  Serial.print(",Max:");   Serial.print(_DIST_MAX);
  Serial.println("");
 
  // update last sampling time
  last_sampling_time += INTERVAL;
}

// -------------------- 거리 측정 함수 --------------------
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  // echo 펄스 길이(μs) → 거리(mm)
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}
