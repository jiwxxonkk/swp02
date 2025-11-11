#define PIN_IR A0

// ---------------------- freeMemory() 구현 ----------------------
extern int __heap_start, *__brkval;
int freeMemory() {
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

// ---------------------- 원본 함수 ----------------------
unsigned int ir_sensor_filtered(unsigned int n, float position, int verbose)
{
  if (n == 0 || n > 100 || position < 0.0 || position > 1.0)
    return 0;

  unsigned long start_time = micros();   // 수행시간 측정 시작
  int mem_before = freeMemory();         // 함수 실행 전 메모리 상태

  // 🔹 malloc으로 메모리 동적할당
  int *samples = (int *) malloc(sizeof(int) * n);
  if (samples == NULL) {
    Serial.println("[Original] Memory allocation failed!");
    return 0;
  }

  int mem_after_alloc = freeMemory();    // 메모리 할당 후 상태
  int mem_used = mem_before - mem_after_alloc;

  // n번 측정
  for (int i = 0; i < n; i++) {
    samples[i] = analogRead(PIN_IR);
    delay(2);
  }

  // 정렬 (버블 정렬)
  for (int i = 0; i < n - 1; i++) {
    for (int j = i + 1; j < n; j++) {
      if (samples[i] > samples[j]) {
        int temp = samples[i];
        samples[i] = samples[j];
        samples[j] = temp;
      }
    }
  }

  int idx = (int)(position * (n - 1));
  int result = samples[idx];

  free(samples); // 🔹 메모리 해제
  int mem_after_free = freeMemory(); // 해제 후 상태

  unsigned long elapsed = micros() - start_time;

  Serial.print("[Original]  Time: ");
  Serial.print(elapsed);
  Serial.print(" us,  Memory Used: ");
  Serial.print(mem_used);
  Serial.print(" bytes (freed -> ");
  Serial.print(mem_after_free - mem_after_alloc);
  Serial.println(" bytes)");

  return result;
}

// ---------------------- 개선형 빠른 버전 (EMA) ----------------------
unsigned int ir_sensor_filtered_fast(unsigned int n, float alpha)
{
  if (n == 0 || n > 100) return 0;

  unsigned long start_time = micros();
  int mem_before = freeMemory();

  float ema = analogRead(PIN_IR);  // 초기값
  for (int i = 1; i < n; i++) {
    float sample = analogRead(PIN_IR);
    ema = alpha * ema + (1.0 - alpha) * sample;
  }

  int mem_after = freeMemory();
  unsigned long elapsed = micros() - start_time;

  Serial.print("[Fast EMA]   Time: ");
  Serial.print(elapsed);
  Serial.print(" us,  Memory Used: ");
  Serial.print(mem_before - mem_after);
  Serial.println(" bytes");

  return (unsigned int)ema;
}

// ---------------------- setup & loop ----------------------
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== IR Sensor Filtered Function Performance Test ===");
}

void loop() {
  ir_sensor_filtered(10, 0.5, 0);     // 원본(정렬 방식, malloc 기반)
  ir_sensor_filtered_fast(10, 0.7);   // 빠른 EMA 방식
  Serial.println("------------------------------------");
  delay(1000);
}
