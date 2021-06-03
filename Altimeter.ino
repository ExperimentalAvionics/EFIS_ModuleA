void Altimeter()
{

 //send a request
Wire.beginTransmission(HONEYWELL_I2C); 
Wire.write(1);  // send a bit asking for register one, the data register
Wire.endTransmission(); 
// now get the data from the sensor
delay (20);

Wire.requestFrom(HONEYWELL_I2C, 4);
while(Wire.available() == 0);
byte a     = Wire.read(); // first received byte 
byte b     = Wire.read(); // second received byte 
byte c     = Wire.read(); // third received byte
byte d     = Wire.read(); // fourth received byte 

  PressureSum -= PressureArray[AltArrayIndex];

byte status1 = (a & 0xc0) >> 6;  // first 2 bits from first byte

int bridge_data = ((a & 0x3f) << 8) + b;

temperature_data = (int)(c)*8;

//           if ( temperature_data == 65535 ) {
//               Serial.println("Static pressure sensor is missing");
//           }
           if ( status1 == 1 ) {
               Serial.println("Warning: Command Mode");// *Command mode is used for programming the sensor. This mode should not be seen during normal operation.
              // Serial.println(status, BIN);   
           }
           if ( status1 == 2 ) {    
               Serial.println("Warning: Stale data");  // if data has already been feched since the last measurement cycle
              // Serial.println(status, BIN);
           }   
           if ( status1 == 3) {
               Serial.println("Static Pressure Sensor fault"); //When the two status bits are "11", one of the above mentioned diagnostic faults is indicated.
              // Serial.println(status, BIN);
           }

   pressure = 1.0 * (bridge_data - OUTPUT_MIN) * (PRESSURE_MAX - PRESSURE_MIN) / (OUTPUT_MAX - OUTPUT_MIN) + PRESSURE_MIN;

   PressureArray[AltArrayIndex] = pressure;
   PressureSum += PressureArray[AltArrayIndex];

  AltArrayIndex +=1;                         // shift the index for next time
  if (AltArrayIndex == AltArraySize) {      // if we reached the top of the array
    AltArrayIndex = 0;                       //go to the start of the array
   
    
  }

   pressure = PressureSum/AltArraySize;
   Altitude = (float)44330 * (1 - pow(((float)pressure/((float)QNH * 100.0)), 0.190295)) * 3.281;

   if (VSIperiod < millis() - VSIlast) {
     VSIlast = millis();
        
//     SumAlt = SumAlt - VSIArray[VSIArrayIndex] + Altitude;
//     SumTime = SumTime - VSITimeArray[VSIArrayIndex] + VSIlast;
//     SumTimeSquare = SumTimeSquare - VSITimeArray[VSIArrayIndex] * VSITimeArray[VSIArrayIndex] + VSIlast * VSIlast;
//     SumTimeAlt = SumTimeAlt - VSIArray[VSIArrayIndex]*VSITimeArray[VSIArrayIndex] +  Altitude * VSIlast;

     VSIArray[VSIArrayIndex] = Altitude;
     VSITimeArray[VSIArrayIndex] = VSIlast;
     
// linear regression 
// we recalsulate SUM's every time we need to do the regression
// the reason is that time values become too large for rolling sum
// so the timescale needs to be shifted left by the smallest time value in the array

      TimeShift=VSITimeArray[0];
      for(i=0; i < VSIArraySize; i++){
        if (TimeShift > VSITimeArray[i]) {
           TimeShift = VSITimeArray[i];
        }
      }

      SumAlt = 0;
      SumTime = 0;
      SumTimeSquare = 0;
      SumTimeAlt = 0;
      for(i=0; i < VSIArraySize; i++){
        SumAlt += VSIArray[i];
        SumTime = SumTime + VSITimeArray[i] - TimeShift;
        SumTimeSquare += (VSITimeArray[i]-TimeShift)*(VSITimeArray[i]-TimeShift);
        SumTimeAlt += VSIArray[i]*(VSITimeArray[i]-TimeShift);
      }
/*
     VSIFirstIndex = VSIArrayIndex + 1;
     if (VSIFirstIndex == VSIArraySize) {
       VSIFirstIndex = 0;
     }
*/

     VSI = (VSIArraySize*SumTimeAlt - SumTime * SumAlt)*60000/(VSIArraySize*SumTimeSquare-SumTime*SumTime);

     VSIArrayIndex +=1;
     if (VSIArrayIndex == VSIArraySize) {
        VSIArrayIndex = 0;
     }
   
   
   }
   

//   VSI = (AltArraySize*PressureTimeSum - TimeSum * PressureSum)*60000/(AltArraySize*TimeSquareSum - TimeSum * TimeSum);
//   VSI = (float)44330 * (1 - pow(((float)VSI/((float)QNH * 100.0)), 0.190295)) * 3.281;

}
