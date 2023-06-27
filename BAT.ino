void Get_BAT() {
// Read backup battery data
long int Vo;
float R2 = 10000;
float logR1, R1, T;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

long int Rv1 = 2200; // Battery voltage divider in kOhm - top resistor
long int Rv2 = 680;  // Battery voltage divider in kOhm - bottom resistor

// Battery temperature
// low temperature threshold and high-temperature threshold in the charger controller are preset internally to 73.5%·VREF33 (0C)and 29.5%·VREF33 (48C)


  Vo = analogRead(A7);
  //R1 = Vo*R2/(1023-(float)Vo);  // 1023 if reference voltage is 5v
  R1 = Vo*R2/(675-(float)Vo);  // 675 if reference voltage is 3.3v
  logR1 = log(R1);
  T = (1.0 / (c1 + c2*logR1 + c3*logR1*logR1*logR1)); // https://en.wikipedia.org/wiki/Steinhart%E2%80%93Hart_equation
  T = T - 273.15;
  //T = T * 9.0 / 5.0 + 32.0; // Fahrenheit conversion

  BAT_Temperature = T;

  Vo = analogRead(A6);
  Vo = Vo*5000/1023;
  Vo = Vo*(Rv1+Rv2)/Rv2;

  BAT_Voltage = Vo; // battery voltage in millivolts



// status information
// ACOK Valid Input Supply Indicator. Open drain output. Add pull-up resistor. Logic LOW indicates the presence of a valid input supply.
// CHGOK Charging Status Indicator. Open drain output. Add pull-up resistor. Logic LOW indicates normal charging. Logic HIGH indicates either a completed charge process or suspended
// process because of some fault.

  BAT_Status = !digitalRead(9); // ACOK
  BAT_Status = BAT_Status*2 + !digitalRead(7); // CHGOK 


/*
  Serial.print("BAT_Voltage = ");
  Serial.println(BAT_Voltage);
  Serial.print("BAT_Temperature = ");
  Serial.println(BAT_Temperature);
  Serial.print("BAT_Status = ");
  Serial.println(BAT_Status);
*/  
  
  }
