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

  PressureFilter();

  if ((millis() - VSI_Timer1) >100) {
       VSI_Timer2 = millis();
       Alt2 = (float)44330 * (1 - pow(((float) pressure/((float) QNH * 100)), 0.190295)) * 3.281;
       VSI = ((Alt2 - Alt1) * 1000.0 * 60.0)/(VSI_Timer2 - VSI_Timer1); 
       vsiFilter();
//       Serial.print("Alt1 = ");
//       Serial.print(Alt1);
//       Serial.print(" Alt2 = ");
//       Serial.print(Alt2);
//       Serial.print(" dT= ");
//       Serial.print(VSI_Timer2 - VSI_Timer1);
//       Serial.print(" VSI = ");
//       Serial.println(VSI);
       // cheat
       if (abs(VSI)<60) {
        VSI=0;
       }
       Alt1 = Alt2;
       VSI_Timer1 = VSI_Timer2;
  }

//           Serial.print("status      ");
//           Serial.println(status1, BIN);
//           Serial.print("pressure  RAW  (Pa) ");
//           Serial.println(pressure);
//           Serial.print("Temperature  RAW ");
//           Serial.println((temperature_data*200/2047)-50);
//           Serial.print("c = ");
//           Serial.println(c);
//           Serial.print("d = ");
//           Serial.println(d);
//           Serial.print(",");
//           PressureFilter();
//         Serial.print("pressure  Filtered  (Pa) ");
//           Serial.print(pressure);
           
//          Altitude = (float)44330 * (1 - pow(((float) pressure/((float) QNH * 100)), 0.190295)) * 3.281;
//          Serial.print(",");
//          Serial.println(Altitude);




}

void PressureFilter() {
// Simple Kalman filter

Pr_p = Pr_p + Pr_q;

Pr_k = Pr_p / (Pr_p + Pr_r);
Pr_x = Pr_x + Pr_k * (pressure - Pr_x);
Pr_p = (1 - Pr_k) * Pr_p;
pressure = Pr_x; 
  
}

void vsiFilter() {
// Simple Kalman filter

VSI_p = VSI_p + VSI_q;

VSI_k = VSI_p / (VSI_p + VSI_r);
VSI_x = VSI_x + VSI_k * (VSI - VSI_x);
VSI_p = (1 - VSI_k) * VSI_p;
VSI = VSI_x; 
  
}
