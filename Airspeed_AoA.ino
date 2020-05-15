// Differential pressure sensor:
//     MPXV7002 - 2 kPa 
//     good for up to 110Kts if used as an airspeed sensor
//     With 60deg betwwen pitot and AOA ports it is good up to 220Kts
// or  MPXV5010 - 10 kPa (~248 Knots)
//     This one might be a bit too rough for the AoA sensor but good for Airspeed sensor
//
// For details visit: http://experimentalavionics.com/angle-of-attack-standalone-unit/

// V=SQRT(2*P/R) where P - dinamic pressure, R is air density (1.225kg/m3)

void Get_Airspeed() {

  int AirspeedSensorValue = 0;
  int AOASensorValue = 0;
  int AirspeedSensorShift = 51;
  int AirspeedSensorMax = 1000;
  float AirDensity = 1.2; //kg/m^3
  float AirPressure = 0;
  float Convert = 1.94; // convert meter per seconds to knots


  AirspeedSensorValue = analogRead(AirspeedPin);
  AOASensorValue = analogRead(AOAPin);

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

void Get_AoA() {


  
}
