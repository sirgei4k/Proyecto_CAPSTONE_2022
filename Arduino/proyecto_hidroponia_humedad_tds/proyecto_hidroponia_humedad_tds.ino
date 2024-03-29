#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;  //configuracion del LCD RGB
const int colorR = 255;
const int colorG = 0;
const int colorB = 0;
const int pinTemp = 2;//pin de sensor de temperatura
const int B = 3975; //constante del sensor de temperatura
float valor_temperatura=0;

int pin_sensor_humedad=3;
//variables del sensor ph
float calibration_value = 21.34+3.92; //se añade el segundo valor como ajuste
int phval = 0; 
unsigned long int avgval; 
int buffer_arr[10],temp;
float val_ph=0;

//Sensor conductividad
#define TdsSensorPin A1
#define VREF 5.0              //  Referencia analogica de 5 V(Volt) del ADC
#define SCOUNT  30            // suma de muestras
int analogBuffer[SCOUNT];     // arreglo para almacenar los valores analogicos de ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;

float averageVoltage = 0;
float tdsValue = 0;
float temperature = 16;       // temperatura de compensación

// algoritmo de filtro de mediana
int getMedianNum(int bArray[], int iFilterLen){
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
  bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0){
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}




void setup() {
  
lcd.begin(16, 2);
    
    lcd.setRGB(colorR, colorG, colorB);
    
    
    lcd.print("Bienvenido");delay(1000);
    Serial.begin(9600);
    pinMode(TdsSensorPin,INPUT);
    
}

void loop() {
  
int humedad=analogRead(pin_sensor_humedad);
funcion_tds();
funcion_ph();
temperatura();
//Serial.print("valor del ph:");
//Serial.println(val_ph);
//Serial.println(humedad);//nivel del agua
//Serial.print("valor del tds:");
//Serial.println(tdsValue);
//Serial.print("la temperatura es: ");
//Serial.println(valor_temperatura);
//estructura del envio al puerto serie
//ph,humedad,tds,temperatura
Serial.print(val_ph);Serial.print(",");Serial.print(humedad);
Serial.print(",");Serial.print(tdsValue);Serial.print(",");Serial.println(valor_temperatura);
delay(1000);


}

float funcion_tds(){
  static unsigned long analogSampleTimepoint = millis();
  if(millis()-analogSampleTimepoint > 40U){     //sensa cada 40 mili segundos el ADC
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //lee el valor analogico y lo almacena en el buffer
    analogBufferIndex++;
    if(analogBufferIndex == SCOUNT){ 
      analogBufferIndex = 0;
    }
  }   
  
  static unsigned long printTimepoint = millis();
  if(millis()-printTimepoint > 800U){
    printTimepoint = millis();
    for(copyIndex=0; copyIndex<SCOUNT; copyIndex++){
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      
      //Lees el valor analogico mas estable por el filtro de mediana convierte a volts
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0;
      
      // compensa por temperatura formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0)); 
      float compensationCoefficient = 1.0+0.02*(temperature-25.0);
      //compensacion por temperatura
      float compensationVoltage=averageVoltage/compensationCoefficient;
      
      //voltage a valalor del tds
      tdsValue=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5;
     // conductividad=tdsValue;
      //Serial.print("voltage:");
      //Serial.print(averageVoltage,2);
      //Serial.print("V   ");
     // Serial.print("TDS Value:");
     // Serial.print(tdsValue,0);
    //  Serial.println("ppm");
    }
  }
}

float funcion_ph() {
 for(int i=0;i<10;i++) 
 { 
 buffer_arr[i]=analogRead(A0);
 delay(30);
 }
 for(int i=0;i<9;i++)
 {
 for(int j=i+1;j<10;j++)
 {
 if(buffer_arr[i]>buffer_arr[j])
 {
 temp=buffer_arr[i];
 buffer_arr[i]=buffer_arr[j];
 buffer_arr[j]=temp;
 }
 }
 }
 avgval=0;
 for(int i=2;i<8;i++)
 avgval+=buffer_arr[i];
 float volt=(float)avgval*5.0/1024/6;
 float ph_act = -5.70 * volt + calibration_value;
 lcd.setCursor(0, 0);
 lcd.print("pH Val:");
 lcd.setCursor(8, 0);
 lcd.print(ph_act);
 delay(1000);
 return val_ph=ph_act;
}

float temperatura(){
  int val = analogRead(pinTemp);
  float resistance = (float)(1023-val)*10000/val;
valor_temperatura = 1/(log(resistance/10000)/B+1/298.15)-273.15;
//return valor_temperatura=valor_temperatura;
}
