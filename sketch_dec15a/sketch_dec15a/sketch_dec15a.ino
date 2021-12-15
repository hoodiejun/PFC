const int analogIn = A0;              
float sensor_Value = 0;              
float v_Out;                         
        
void setup() {
  Serial.begin (9600);                
}

void loop() {
  sensor_Value = analogRead (A0);     // read the analog in value:
  v_Out = 250 * (sensor_Value / 1024)*sqrt(3);
 
  Serial.print ("vOut = ");
  Serial.print(v_Out);
  Serial.println("V");

  delay (1000);
}
