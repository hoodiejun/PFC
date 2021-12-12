#include <LiquidCrystal_I2C.h>
#include <Wire.h> 
#include <Filters.h>              //This library does a huge work check its .cpp file

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define Voltage_Pin A0 // ZMPT101b data pin
#define Current_Pin A1 // ACS712 data pin
#define ZeroCrossing_Pin 13 // Zero Crossing data pin
#define Relay1_Pin 12 // Relay switch 1
#define Relay2_Pin 11 // Relay switch 2

float rads = 57.29577951; // 1 radian
float degree = 360;
float frequency = 60;
float micro = 1 * pow (10,-6); // Multiplication factor to convert micro seconds into seconds

float pf;
float angle;
float pf_max = 0;
float angle_max = 0;
int ctr;
int capState1 = 0;
int capState2 = 0;

float Amps_TRMS; 
float ACS_Value;
unsigned long printPeriod = 1000; 
unsigned long previousMillis = 0;

void setup()
{
  pinMode(ZeroCrossing_Pin, INPUT);
  pinMode(Relay1_Pin, OUTPUT);
  pinMode(Relay2_Pin, OUTPUT);
  digitalWrite(Relay1_Pin,LOW);
  digitalWrite(Relay2_Pin,LOW);
  Serial.begin(9600);
  Serial.println("Serial started");
  inputStats.setWindowSecs( windowLength );
  lcd.begin(16, 2);  // 16행(0~15) 2열(0~1) LCD 표시
  lcd.backlight(); // 백라이트 ON
  lcd.home ();
}

void loop()
{
  ReadVoltage();
  ReadCurrent();
  CalcPF();
  ControlRelay();

  // lcd 출력
  lcd.setCursor(0, 0); // 1행 1열부터 시작
  lcd.print("pf=");
  lcd.print(pf);
  lcd.setCursor(7, 0);
  lcd.print("P=");
  lcd.print(Volts_TRMS*Amps_TRMS);
  lcd.setCursor(13,0);
  lcd.print("kW");
  lcd.setCursor(0, 1);
  lcd.print("Q=");
  lcd.print(Volts_TRMS*Amps_TRMS*sin(angle_max));
  lcd.setCursor(5,1);
  lcd.print("kVAR");

  delay(2000);
  angle = 0; // 다음 측정을 위해 각도 0으로 리셋
  angle_max = 0;
}

float ReadCurrent()
{
  float windowLength = 40.0/frequency; // how long to average the signal, for statistist
  float intercept = 0; // to be adjusted based on calibration testing
  float slope = 0.0752; // to be adjusted based on calibration testing
  
  RunningStatistics inputStats; // create statistics to look at the raw test signal
  inputStats.setWindowSecs( windowLength );
   
  ACS_Value = analogRead(Current_Pin); // read the analog in value:
  inputStats.input(ACS_Value); // log to Stats function
      
  if((unsigned long)(millis() - previousMillis) >= printPeriod) { //every second we do the calculation
    previousMillis = millis(); // update time
    
    Amps_TRMS = intercept + slope * inputStats.sigma();  //Calibrate the values
    lcd.clear(); //clear the lcd and print in a certain position
    lcd.setCursor(2,0);
    lcd.print(Amps_TRMS);
    lcd.print(" A");
  }
}

float windowLength = 100/frequency; // how long to average the signal, for statistist, changing this can have drastic effect
int RawValue = 0;     
float Volts_TRMS; // estimated actual voltage in Volts

float intercept = 0; // to be adjusted based on calibration testin
float slope = 1;      

unsigned long printPeriod = 1000; //Measuring frequency, every 1s, can be changed
unsigned long previousMillis = 0;

RunningStatistics inputStats; //This class collects the value so we can apply some functions

void setup() {
  Serial.begin(115200);    // start the serial port
  Serial.println("Serial started");
  inputStats.setWindowSecs( windowLength );
}

float ReadVoltage()
{
    RawValue = analogRead(Voltage_Pin);  // read the analog in value:
    inputStats.input(RawValue);       // log to Stats function
        
    if((unsigned long)(millis() - previousMillis) >= printPeriod) { //We calculate and display every 1s
      previousMillis = millis();   // update time
      
      Volts_TRMS = inputStats.sigma()* slope + intercept;
//      Volts_TRMS = Volts_TRMS*0.979;              //Further calibration if needed
      
      Serial.print("Non Calibrated: ");
      Serial.print("\t");
      Serial.print(inputStats.sigma()); 
      Serial.print("\t");
      Serial.print("Calibrated: ");
      Serial.print("\t");
      Serial.println(Volts_TRMS, 2);
    
  }
}

float CalcPF()
{
  for (ctr = 0; ctr <= 2; ctr++) // 4번 측정 수행하고 리셋
  {
    // Cos 함수가 라디안을 사용하므로 angle/57.2958로 변환시켜줌
    angle = ((((pulseIn(ZeroCrossing_Pin, HIGH)) * micro)* degree)* frequency); // 차별화된 시간 펄스로부터 각도로 위상각을 계산한다
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
  // 시리얼 모니터 출력
  Serial.print("위상차: ");
  Serial.print(angle_max, 2);
  Serial.print(",");
  Serial.print("역률: ");
  Serial.println(pf_max, 2);
}

float ControlRelay()
{
  // 역률 0.95이하면 릴레이 ON 
  if(pf_max <= 0.95 && capState1 == 0)
  {
    digitalWrite(Relay1_Pin,LOW);
    delay(100);
    capState1 = 1;
  }
  else if(pf_max <= 0.95 && capState1 == 1 && capState2 ==0)
  {
    digitalWrite(Relay2_Pin,LOW);
    delay(100);
    capState2 = 1;
  }
}