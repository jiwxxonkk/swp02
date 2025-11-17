// Arduino pin assignment
#define PIN_IR A0

float volt[20];
float dist[20];
int idx = 0;
int N = 0;

float A[10][10];
float B[10];
float X_coef[10];

float volt_to_distance(unsigned int a_value) 
{
  // Replace below line with the equation obtained from nonlinear regression analysis
  return (6762.0 / (a_value - 9) - 4.0) * 10.0;
}

int compare(const void *a, const void *b) {
  return (*(unsigned int *)a - *(unsigned int *)b);
}

unsigned int ir_sensor_filtered(unsigned int n, float position, int verbose)
{
  unsigned int *ir_val, ret_val;
 
  if (n == 0 || n > 200) return 0;
  if (position > 1.0) position = 1.0;
  if (position == 1.0) position = 0.999;

  ir_val = (unsigned int *)malloc(sizeof(unsigned int) * n);
  if (ir_val == NULL) return 0;

  for (int i = 0; i < n; i++) {
    ir_val[i] = analogRead(PIN_IR);
  }

  qsort(ir_val, n, sizeof(unsigned int), compare);
  ret_val = ir_val[(unsigned int)(n * position)];

  free(ir_val);
  return ret_val;
}

// 연립방정식 해 풀기 
void gaussian(int n) {
  for (int i = 0; i < n; i++) {

    if (A[i][i] == 0) {
      for (int r = i + 1; r < n; r++) {
        if (A[r][i] != 0) {
          for (int c = 0; c < n; c++) {
            float tmp = A[i][c];
            A[i][c] = A[r][c];
            A[r][c] = tmp;
          }
          float tmp2 = B[i];
          B[i] = B[r];
          B[r] = tmp2;
          break;
        }
      }
    }

    float pivot = A[i][i];
    if (pivot == 0) continue;

    for (int c = 0; c < n; c++) A[i][c] /= pivot;
    B[i] /= pivot;

    for (int r = i + 1; r < n; r++) {
      float factor = A[r][i];
      for (int c = 0; c < n; c++) {
        A[r][c] -= factor * A[i][c];
      }
      B[r] -= factor * B[i];
    }
  }

  for (int i = n - 1; i >= 0; i--) {
    float sum = B[i];
    for (int c = i + 1; c < n; c++) sum -= A[i][c] * X_coef[c];
    X_coef[i] = sum;
  }
}

// 다항식 curve fitting을 실제로 수행하는 핵심 함수 
void polynomialRegression(int degree) {
  int m = degree + 1;

  for (int i = 0; i < m; i++) {
    for (int j = 0; j < m; j++) A[i][j] = 0;
    B[i] = 0;
  }

  float S[20] = {0};

  for (int k = 0; k <= 2 * degree; k++) {
    float sum = 0;
    for (int i = 0; i < N; i++) sum += pow(volt[i], k);
    S[k] = sum;
  }

  for (int r = 0; r < m; r++)
    for (int c = 0; c < m; c++)
      A[r][c] = S[r + c];

  for (int r = 0; r < m; r++) {
    float sum = 0;
    for (int i = 0; i < N; i++)
      sum += dist[i] * pow(volt[i], r);
    B[r] = sum;
  }

  gaussian(m);
}

void setup()
{
  Serial.begin(1000000);  // initialize serial port
  delay(1000);
  // 차수 입력받기 
  Serial.print("Enter Polynomial degree (1~5): ");
  while (!Serial.available());
  // 입력받은 차수 degree 변수에 저장 
  int degree = Serial.parseInt();
  if (degree < 1) degree = 1;
  if (degree > 5) degree = 5;

  Serial.print("Selected Polynomial degree : ");
  
  Serial.println(degree);

  Serial.println("\nPress Enter after moving ball.\n");

  for (int d = 0; d <= 30; d += 5) {
    Serial.print(d); Serial.println("Press Enter.");

    while (!Serial.available());
    // 엔터 키가 들어올 때까지 기다림 , 엔터를 받으면 바로 센서를 측정함 
    Serial.read();
    // 노이즈 제거, n번 반복 측정, 정렬해서 중간값 반환
    
    unsigned int filtered = ir_sensor_filtered(50, 0.5, 0);
    // 그 값을 volt[] 배열에 저장하고, 동시에 해당 거리도 dist[]에 저장해서 curve fitting 할 수 있는 데이터 테이블 만듦 
    
    volt[idx] = filtered;
    dist[idx] = d * 10.0;
    idx++;
    N++;
    
    Serial.print("ADC = ");
    Serial.println(filtered);
}

  polynomialRegression(degree);
  
  Serial.print("distance = "); // 최종 equation 출력 
  for (int i = 0; i <= degree; i++) {
    Serial.print(X_coef[i], 6);
    if (i >= 1) Serial.print("*v");
    if (i >= 2) {
      for (int k = 2; k <= i; k++) Serial.print("*v");
    }
    if (i != degree) Serial.print(" + ");
  }
  Serial.println(";");
  Serial.println("you can use above equation into volt_to_distance()");
} // end setup()

void loop() {
  // do nothing
}
