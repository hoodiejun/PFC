#define ADC_PIN A0
#define ADCI_PIN A1
#define SAMPLING 1000

#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>
modbusDevice regBank;
modbusSlave slave;

#define RS485TxEnablePin 2
#define RS485Baud 9600
#define RS485Format SERIAL_8E1


float Ianalog=0,adc_max,adc_min,adc_vpp,vout,value,prev;

//Variable Decleration For RMS & Inst 
float Iinst,Vinst;
double Isq,Vsq;
double Iavg,Vavg;
double Irms,Vrms;
double Prms,Pinst,PF;
double b = 0;
float VoltLevel = 3.72;

void setup()
{
  
  //Assign the modbus device ID.  
  regBank.setId(1);

  Serial.begin(9600);
  Serial1.begin(9600);

  //Add Analog Input registers to the register bank
  regBank.add(30001);  
  regBank.add(30002);  
  regBank.add(30003);  
  regBank.add(30004);  
  regBank.add(30005);  
  regBank.add(30006);  
  regBank.add(30007);  
  regBank.add(30008);  
  regBank.add(30009);  
  regBank.add(30010);  
  regBank.add(30011);  
  regBank.add(30012);  
  regBank.add(30013);   

//Add Analog Output registers to the register bank
  regBank.add(40001);  
  regBank.add(40002);
  slave._device = &regBank;  
  slave.setBaud(&Serial1,RS485Baud,RS485Format,RS485TxEnablePin);   
}

void loop()
{
  int a = 0;
   adc_max=0;
   adc_min=1024;
   for(int cnt=0;cnt<SAMPLING;cnt++)
      {
        //Caluclation For Instantaneous Current
        
          Ianalog = analogRead(A1);
         
          if(Ianalog > adc_max)
          {
            adc_max = Ianalog;
          }
          if(Ianalog < adc_min)
          {
            adc_min = Ianalog;
          }
          Iinst = abs((((adc_max-adc_min)/10023))*20*7.727-0.04);
          
          Pinst += ((5.0/1023.0)*analogRead(A0)*(220/VoltLevel))*Iinst;
       
        //Caluclation For Sum Of Square Current
        Isq += sq( Iinst);        
        //Caluclation For Instantaneous Voltage
        vout  = (5.0/1023.0)*analogRead(A0);
        Vinst = (vout*220)/VoltLevel;
        //Caluclation For Sum Of Square Voltage
        Vsq += sq(Vinst);        
        //Caluclation For Instantaneous Power                                                                   
          
        //Serial.println(String ("Pinst = ")+Vinst + " * " + Iinst + " = "+Vinst*Iinst);
      }
        
        
        //Caluclation For RMS Current
        Iavg = Isq/SAMPLING;
        Irms = sqrt(Iavg);
        
        //Caluclation For RMS Voltage
        Vavg = Vsq/SAMPLING;
        Vrms = sqrt(Vavg);

        //Caluclation For RMS Power
        Prms = Vrms*Irms;

        //Caluclation For Inst Power
        Pinst = Pinst/SAMPLING;
        
        //False Current Reading
        if (Prms < 18 && Pinst < 18)
        {
          PF=0;
          Prms = 0;
          Pinst = 0;
        }
        else        
        
        //Caluclation For Power Factor
        PF = Pinst/Prms;

        if(PF == 1)
        PF = 0;
    
        if (b>2)
        {
        Serial.println();
        Serial.println("Energy Analyzer");
        Serial.println("---------------");
        Serial.println();
        Serial.println(String("Instantaneous Voltage = ")+ Vinst + " Vac");
        Serial.println(String("Instantaneous Current = ")+ Iinst + " Iac");
        Serial.println(String("Average Voltage = ")+ Vavg + " Vac");
        Serial.println(String("Average Current = ")+ Iavg + " Iac");
        Serial.println(String("RMS Voltage = ")+ Vrms + " Vac");
        Serial.println(String("RMS Current = ")+ Irms + " Amp");
        Serial.println(String("Average Power P = ")+ Pinst + " Watt");
        Serial.println(String("Apparent Power S = ")+ Prms + " VA");
        Serial.println(String("Power Factor PF = ")+ PF + " ");
        Serial.println();
        b=0;
        }
        double Intiger=0,fract=0;
        double Intiger1=0,fract1=0;
        double Intiger2=0,fract2=0;
        double Intiger3=0,fract3=0;
        //fract = 0;Intiger=0;
        //fract= modf (Vrms, &Intiger);
        //fract = fract*100;
        regBank.set(30001, Vrms); //from 0 - 1023
        //regBank.set(30002, (word) fract); //from 0 - 1023
     
       
        fract= modf (Irms, &Intiger);
        fract = fract*100;
        regBank.set(30003, Intiger); //from 0 - 1023
        regBank.set(30005, fract); //from 0 - 1023
        

        fract2= modf (Prms, &Intiger2);
        fract2 = fract2*100;
        regBank.set(30007,Intiger2); //from 0 - 1023
        regBank.set(30009,fract2); //from 0 - 1023

        fract3= modf (PF, &Intiger3);
        fract3 = fract3*100;
        regBank.set(30011,Intiger3); //from 0 - 1023
        regBank.set(30013,fract3); //from 0 - 1023

        
        b++;
      Iavg=0;
      prev=0;
      Vsq = 0;
      Isq = 0;
      Pinst = 0;
  slave.run();  
  delay(100);
}
