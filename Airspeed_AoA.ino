void Airspeed_AoA() {

  int AirspeedSensorValue = 0;
  int AirspeedSensorShift = 51;
  int AirspeedSensorMax = 1000;
  float AirDensity = 1.2; //kg/m^3
  float AirPressure = 0;
  float Convert = 1.94; // convert meter per seconds to knots


  AirspeedSensorValue = analogRead(AirspeedPin);


// Serial.print("Analog Input = ");
// Serial.println(AirspeedSensorValue);

  AirPressure = map(AirspeedSensorValue, AirspeedSensorShift, AirspeedSensorMax, 0, 10000);
 
 if (AirPressure<0) {
  AirPressure = 0;
 }

// Serial.print("Pressure = ");
// Serial.println(AirPressure);
 
 Airspeed = Convert * sqrt(2*AirPressure/AirDensity);

 if (Airspeed < 15) {
  Airspeed = 0;
 }
// Serial.print("Airspeed = ");
// Serial.println(Airspeed);

}

