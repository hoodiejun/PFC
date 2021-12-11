#include <LiquidCrystal.h>

#define Voltage_Pin A0
#define Current_Pin A1
#define ZeroCrossing_Pin 13
#define Relay1_Pin 12
#define Relay2_Pin 11

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
float rads = 57.29577951; // 1 radian
float degree = 360;
float frequency = 10;
float micro = 1 * pow (10,-6); // Multiplication factor to convert micro seconds into seconds

float pf;
float angle;
float pf_max = 0;
float angle_max = 0;
int ctr;
int capState1 = 0;
int capState2 = 0;

void setup()
{
  pinMode(ZeroCrossing_Pin, INPUT);
  pinMode(Relay1_Pin, OUTPUT);
  pinMode(Relay2_Pin, OUTPUT);
  digitalWrite(Relay1_Pin,LOW);
  digitalWrite(Relay2_Pin,LOW);
  Serial.begin(9600);
  lcd.begin(16, 2);
}

void loop()
{
  for (ctr = 0; ctr <= 10; ctr++) // 4번 측정 수행하고 리셋
  {
    // 1st line calculates the phase angle in degrees from differentiated time pulse
    // Cos 함수가 라디안을 사용하므로 angle/57.2958로 변환시켜줌
    angle = (((pulseIn(ZeroCrossing_Pin, HIGH)) * micro)* degree);
    // pf = cos(angle / rads);
    if (angle > angle_max) // 측정 각도가 최댓값 보다 크면
    {
      angle_max = angle; // 각도의 최댓값을 대입
      pf_max = cos(angle_max / rads); // "angle_max" 로 부터 역률을 계산
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

  // 역률 0.98이하면 릴레이 ON 
  if(pf_max <= 0.95 && capState1 == 0)
  {
    digitalWrite(Relay1_Pin,LOW);
    delay(100);
    capState1 = 1;
  }
  else if(pf_max <= 0.98 && capState1 == 1 && capState2 ==0)
  {
    digitalWrite(Relay2_Pin,LOW);
    delay(100);
    capState2 = 1;
  }

  // 시리얼 모니터 출력
  Serial.print("위상차: ");
  Serial.print(angle_max, 2);
  Serial.print(",");
  Serial.print("역률: ");
  Serial.println(pf_max, 2);

  // lcd 출력
  // lcd.clear();
  // lcd.setCursor(0,0);
  // lcd.print("PF=");
  // lcd.setCursor(4,0);
  // lcd.print(pf_max);
  // lcd.print(" ");
  // lcd.setCursor(0,1);
  // lcd.print("Ph-Shift=");
  // lcd.setCursor(10,1);
  // lcd.print(angle_max);
  // lcd.print(" ");

  delay(2000);
  angle = 0; // 다음 측정을 위해 각도 0으로 리셋
  angle_max = 0;
}
