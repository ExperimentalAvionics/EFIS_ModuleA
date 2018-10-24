
#include "Wire.h"
#include <EEPROM.h>
#include <Arduino.h>
#include <mcp_can.h>

// Honeywell barometer ***************************************************************************************
#define HONEYWELL_I2C 0x28 // each I2C object has a unique bus address
                          // HSCDANN015PA2A3 has rabge 0 PSI - 15 PSI (0hPa - 1034.21hPa)
#define OUTPUT_MIN 1638        // 1638 counts (10% of 2^14 counts or 0x0666)
#define OUTPUT_MAX 14745       // 14745 counts (90% of 2^14 counts or 0x3999)
#define PRESSURE_MIN 0        // min is 0 for sensors that give absolute values
#define PRESSURE_MAX 103421   // Pressure in Pascals
// more dteils here https://sensing.honeywell.com/index.php?ci_id=45841

// Kalman filter valiables for Pressure sensor
  float Pr_q = 1; //process noise covariance
  float Pr_r = 1000; //measurement noise covariance
  float Pr_x = 99400; //value
  float Pr_p; //estimation error covariance
  float Pr_k; //kalman gain
  float pressure = 0;
  unsigned long Pressure_Int = 0;  //pressure muliplied by 10 and converted to integer for sending via CAN

// Kalman filter valiables for VSI
  float VSI_q = 1; //process noise covariance
  float VSI_r = 100; //measurement noise covariance
  float VSI_x = 0; //value
  float VSI_p; //estimation error covariance
  float VSI_k; //kalman gain
  int VSI = 0;
  unsigned long VSI_Timer1 = 0;
  unsigned long VSI_Timer2 = 0;
  
  int Alt1 = 0;
  int Alt2 = 0;

  int QNH = 1013; //hPa
  float Altitude = 0; // Altitude in feet


  unsigned int Airspeed = 0;
//  int VerticalSpeed = 0;
  int AoA = 0;

//Airspeed
 int AirspeedPin = A0;

// CAN
const int CAN_CS_PIN = 10;   //CS pin for CAN board

unsigned long LoopCounter = 0;
unsigned long LoopTimer = 0;

unsigned long canId;  

//CAN Message details
unsigned char canMsg[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

//Airspeed (2 bytes), Altitude (3 bytes), Vert Speed (2 bytes), Angle of Attack (1 byte)
const unsigned int CAN_Air_Msg_ID = 40; // CAN Msg ID in DEC
const unsigned int CAN_Air_Period = 200; // How often message sent in milliseconds
unsigned long CAN_Air_Timestamp = 0; // when was the last message sent
unsigned long CAN_QNH_Timestamp = 0;
const unsigned int CAN_QNH_Period = 5000;
int QNH_MemOffset = 0;

unsigned char len = 0;
unsigned char buf[8];
unsigned char ext = 0;

MCP_CAN CAN(CAN_CS_PIN);                                    // Set CS pin

void setup()
{
 Wire.begin(); // wake up I2C bus
 delay (50);
 Serial.begin(115200);

   while (CAN_OK != CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ))              // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
        delay(100);
    }
    CAN.setMode(MCP_NORMAL);
    Serial.println("CAN BUS Shield init ok!");


 QNH = Read_QNH(); 

 Altimeter();
 VSI_Timer1 = millis();
 Alt1 = (float)44330 * (1 - pow(((float) pressure/((float) QNH * 100)), 0.190295)) * 3.281;;
 Alt2 = (float)44330 * (1 - pow(((float) pressure/((float) QNH * 100)), 0.190295)) * 3.281;;
  
}



void loop()
{

unsigned int AltitudeINT;

// see if there are config messages available, like QNH



if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
      CAN.readMsgBuf(&canId, &ext, &len, buf);    // read data,  len: data length, buf: data buf
      switch (canId) {
          case 46:
            
            QNH = (buf[1] << 8) | buf[0];

            if (Read_QNH() != QNH) {
              Write_QNH();
              CAN_QNH_Timestamp = millis();
              Serial.print("New QNH is written: ");
              Serial.println(QNH);
            }
            break;
          default: 
          break;
        }
}





 Altimeter();
 Airspeed_AoA();

// Send Air data
if (millis() > CAN_Air_Timestamp + CAN_Air_Period + random(0, 50)) {

  Pressure_Int = pressure * 100.0;
  
          Serial.print(pressure);
          Altitude = (float)44330 * (1 - pow(((float) pressure/((float) QNH * 100)), 0.190295)) * 3.281;
          Serial.print(",");
          Serial.print(Pressure_Int);
          Serial.print(",");
          Serial.println(Altitude);

          Serial.print("===================== QNH: ");
          Serial.println(QNH);


          AltitudeINT = Altitude;

  canMsg[0] = Airspeed;
  canMsg[1] = Airspeed >> 8;
  
  canMsg[2] = AltitudeINT;
  canMsg[3] = AltitudeINT >> 8;
  canMsg[4] = AltitudeINT >> 16;

  canMsg[5] = VSI;
  canMsg[6] = VSI >> 8;
  
  canMsg[7] = AoA;
  
  CAN.sendMsgBuf(CAN_Air_Msg_ID, 0, 8, canMsg); 
  
  CAN_Air_Timestamp = millis();
}

// QNH is store in this module as well as Display modules where it can be changes.
// Latest change in the Display module overwrites the QNH value on all devices.
// The QNH gets re-broadcasted in case one of the units goes down 
if (millis() > CAN_QNH_Timestamp + CAN_QNH_Period) {

 Send_QNH();
 CAN_QNH_Timestamp = millis();

}


 

 
}



