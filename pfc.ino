#include <LiquidCrystal.h>
int pin = 13;
float rads = 57.29577951; // 1 radian = approx 57 deg.
float degree = 360;
float frequency = 60;
float nano = 1 * pow (10,-6); // Multiplication factor to convert micro seconds into seconds

// Define floats to contain calculations

float pf;
float angle;
float pf_max = 0;
float angle_max = 0;
int ctr;
int capState1 = 0;
int capState2 = 0;
int Relaypin1 = 12;
int Relaypin2 =11;

void setup()
{
 pinMode(pin, INPUT);
 Serial.begin(9600);
 pinMode(Relaypin1, OUTPUT);
 pinMode(Relaypin2, OUTPUT);
 digitalWrite(Relaypin1,LOW);
 digitalWrite(Relaypin2,LOW);
}

void loop()
{
 
for (ctr = 0; ctr <= 10; ctr++) // Perform 4 measurements then reset
  {
  // 1st line calculates the phase angle in degrees from differentiated time pulse
  // Function COS uses radians not Degree's hence conversion made by dividing angle / 57.2958
   angle = ((((pulseIn(pin, HIGH)) * nano)* degree)* frequency);
  // pf = cos(angle / rads);
  
   if (angle > angle_max) // Test if the angle is maximum angle
     {
      angle_max = angle; // If maximum record in variable "angle_max"
      pf_max = cos(angle_max / rads); // Calc PF from "angle_max"
     }
   }
   if (angle_max > 360) // If the calculation is higher than 360 do following...
   {
    angle_max = 0; // assign the 0 to "angle_max"
    pf_max = 1; // Assign the Unity PF to "pf_max"
   }
   if (angle_max == 0) // If the calculation is higher than 360 do following...
   {
    angle_max = 0; // assign the 0 to "angle_max"
    pf_max = 1; // Assign the Unity PF to "pf_max"
   }
   Serial.print(angle_max, 2); // Print the result
   Serial.print(",");
   Serial.println(pf_max, 2);

   if(0 <= pf_max && pf_max <= 0.95)
   {
    digitalWrite(Relaypin1,HIGH);
    delay(200);
    capState1 = 1;
   }
   delay(1000);
   angle = 0; // Reset variables for next test
   angle_max = 0;
}
