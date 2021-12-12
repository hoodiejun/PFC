#include <ACS712.h>
#include <ZMPT101B.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h> 

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define ZeroCrossing_Pin 13
#define Relay1_Pin 12
#define Relay2_Pin 11

ZMPT101B voltageSensor(A0);
ACS712 currentSensor(ACS712_05B, A1);

float rads = 57.29577951; // 1라디안
float degree = 360;
float frequency = 60;
float micro = 1 * pow (10,-6); // 마이크로 초로 변환

float angle = 0;
float pf_max = 0;
float angle_max = 0;
float total_pf = 0;
int ctr = 0;
int capState1 = 0;
int capState2 = 0;

void setup()
{
  pinMode(ZeroCrossing_Pin, INPUT);
  pinMode(Relay1_Pin, OUTPUT);
  pinMode(Relay2_Pin, OUTPUT);
  Serial.begin(9600);
  Serial.println("Serial started");
  voltageSensor.calibrate();
  currentSensor.calibrate();
  Serial.println("Done!");
  lcd.begin(16, 2);  // 16행(0~15) 2열(0~1) LCD 표시
  lcd.backlight(); // 백라이트 ON
  lcd.home ();
  digitalWrite(Relay1_Pin,LOW);
  digitalWrite(Relay2_Pin,LOW);
}

void loop()
{
  for (ctr = 0; ctr <= 4; ctr++) // 5번 측정 수행하고 리셋
  {
    // Cos 함수가 라디안을 사용하므로 angle/57.2958로 변환시켜줌
    angle = ((((pulseIn(ZeroCrossing_Pin, HIGH)) * micro)* degree)* frequency); // 차별화된 시간 펄스로부터 각도로 위상각을 계산한다
    if (angle > angle_max) // 측정 각도가 최댓값 보다 크면
    {
      angle_max = angle; // 각도의 최댓값을 대입
      pf_max = cos(angle_max / rads); // "angle_max" 로 부터 역률을 계산
      total_pf = total_pf + pf_max;
    }
  }
  if (angle_max > 360) // 각도 최댓값이 360 이상이면
  {
    angle_max = 0;
    pf_max = 1;
  }
  if (angle_max == 0)
  {
    angle_max = 0;
    pf_max = 1;
  }
  angle_max = total_pf / ctr;
  
  // 시리얼 모니터 출력
  Serial.print("위상차: ");
  Serial.print(angle_max, 2);
  Serial.print(",");
  Serial.print("역률: ");
  Serial.println(pf_max, 2);
  
  float U = voltageSensor.getVoltageAC();
  if(U < 150)
  {
    U = 0.05;
  }
  float I = currentSensor.getCurrentAC();
  float P = U * I;

  Serial.println(String("V = ") + U + " V");
  Serial.println(String("I = ") + I + " A");
  Serial.println(String("P = ") + P + " W");
  delay(1000);

  // 역률 0.95이하면 릴레이 ON 
  if(0 <= pf_max && pf_max <= 0.95 && capState1 == 0)
  {
    digitalWrite(Relay1_Pin,HIGH);
    Serial.println("릴레이 스위치1 ON");
    delay(100);
    capState1 = 1;
  }
  // 첫 번째 스위치 ON 후 역률 0.95 이하면 두 번째 스위치 ON
  else if(0 <= pf_max && pf_max <= 0.95 && capState1 == 1 && capState2 ==0)
  {
    digitalWrite(Relay2_Pin,HIGH);
    Serial.println("릴레이 스위치2 ON");
    delay(100);
    capState2 = 1;
  }

  // 역률, 유효전력, 무효전력 출력
  lcd.clear();
  lcd.setCursor(0,0); // 1행 1열부터 시작
  lcd.print("PF=");
  lcd.print(pf_max);
  lcd.setCursor(9,0);
  lcd.print("P=");
  lcd.print(P);
  lcd.setCursor(0,1);
  lcd.print("V=");
  lcd.print(U);
  lcd.setCursor(9,1);
  lcd.print("I=");
  lcd.print(I);
  delay(3000);
  lcd.clear();

  // 전압, 전류, 스위치 상태 출력
  if(capState1 == 0)
  {
    lcd.setCursor(0, 0);
    lcd.print("Relay1:");
    lcd.print("OFF");
  }
  else if(capState1 == 1)
  {
    lcd.setCursor(0,0);
    lcd.print("Relay1:");
    lcd.print("ON");
  }
  if(capState2 == 0)
  {
    lcd.setCursor(0,1);
    lcd.print("Relay2:");
    lcd.print("OFF");
  }
  else if(capState2 == 1)
  {
    lcd.setCursor(0,1);
    lcd.print("Relay2:");
    lcd.print("ON");
  }

  delay(500);
  angle = 0; // 다음 측정을 위해 각도 0으로 리셋
  angle_max = 0;
  total_pf = 0;
}
