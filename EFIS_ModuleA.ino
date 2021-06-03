
#include "Wire.h"
#include <EEPROM.h>
#include <Arduino.h>
#include <mcp_can.h>

// Honeywell barometer ***************************************************************************************
#define HONEYWELL_I2C 0x28 // each I2C object has a unique bus address
                          // HSCDANN015PA2A3 has range 0 PSI - 15 PSI (0hPa - 1034.21hPa)
#define OUTPUT_MIN 1638        // 1638 counts (10% of 2^14 counts or 0x0666)
#define OUTPUT_MAX 14745       // 14745 counts (90% of 2^14 counts or 0x3999)
#define PRESSURE_MIN 0        // min is 0 for sensors that give absolute values
#define PRESSURE_MAX 103421   // Pressure in Pascals
// more dteils here https://sensing.honeywell.com/index.php?ci_id=45841

float pressure = 0;
unsigned long Pressure_Int = 0;  //RAW pressure in hPa muliplied by 10 and converted to integer for sending via CAN as 2-byte value
long VSI = 0;

// Moving Average array for Altitude and VSI
#define AltArraySize 60 // averaging accross last 60 values
#define VSIArraySize 15 // averaging accross last 0 values
long PressureArray[AltArraySize];
long VSITimeArray[VSIArraySize];
float VSIArray[VSIArraySize]; // array of values to calculate VSI
int AltArrayIndex = 0;
int VSIArrayIndex = 0;
long PressureSum = 0;
long VSIperiod = 200; // period of logging the altitude for VSI purpose, milliseconds
long VSIlast = 0; // last time we logged altitude for VSI calculation
// int VSIFirstIndex = 0; // temp variable to figure out the earliest value in the VSI array.

// stuff fo linear regression
float  SumAlt = 0; 
unsigned long SumTimeSquare = 0;
unsigned long SumTime = 0;
unsigned long SumTimeAlt = 0;
unsigned long VSITimeSquareArray[VSIArraySize];
unsigned long TimeShift = 0;

int i = 0;


  int QNH = 1013; //hPa
  float Altitude = 0; // Altitude in feet

  int temperature_data=0;
  int temperature_raw = 0;

  int OAT = 0;
  int Humidity = 0;

  unsigned int Airspeed = 0;
//  int VerticalSpeed = 0;
  int AoA = 0;

//Airspeed
 int AirspeedPin = A0;
 int AOAPin = A1;

// CAN
const int CAN_CS_PIN = 10;   //CS pin for CAN board

unsigned long LoopCounter = 0;
unsigned long LoopTimer = 0;

unsigned long canId;  

//CAN Message details
unsigned char canMsg[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

//Airspeed (2 bytes), Altitude (3 bytes), Vert Speed (2 bytes)

const unsigned int CAN_Air_Msg_ID = 40; // CAN Msg ID in DEC
const unsigned int CAN_AoA_Msg_ID = 41; // CAN Msg ID in DEC
const unsigned int CAN_OAT_Msg_ID = 42; // CAN Msg ID in DEC
const unsigned int CAN_RAW_Msg_ID = 43; // CAN Msg ID in DEC
const unsigned int CAN_QNH_Msg_ID = 46; // CAN Msg ID in DEC
const unsigned int CAN_Air_Period = 300; // How often message sent in milliseconds
const unsigned int CAN_AoA_Period = 200; // How often message sent in milliseconds
const unsigned int CAN_OAT_Period = 2000; // How often message sent in milliseconds
const unsigned int CAN_RAW_Period = 500; // How often message sent in milliseconds
const unsigned int CAN_QNH_Period = 5000;
unsigned long CAN_Air_Timestamp = 0; // when was the last message sent
unsigned long CAN_AoA_Timestamp = 0; // when was the last message sent
unsigned long CAN_OAT_Timestamp = 0; // when was the last message sent
unsigned long CAN_RAW_Timestamp = 0; // when was the last message sent
unsigned long CAN_QNH_Timestamp = 0;
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
 VSIlast = millis();
  
}



void loop()
{

unsigned int AltitudeINT;

// see if there are config messages available, like QNH



if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
      CAN.readMsgBuf(&canId, &ext, &len, buf);    // read data,  len: data length, buf: data buf
      switch (canId) {
          case CAN_QNH_Msg_ID:
            
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
 Get_Airspeed();
 Get_AoA();
 

  Pressure_Int = pressure/10;

// Send Air data
if (millis() > CAN_Air_Timestamp + CAN_Air_Period + random(0, 50)) {

  AltitudeINT = Altitude;

  canMsg[0] = Airspeed;
  canMsg[1] = Airspeed >> 8;
  
  canMsg[2] = AltitudeINT;
  canMsg[3] = AltitudeINT >> 8;
  canMsg[4] = AltitudeINT >> 16;

  canMsg[5] = VSI;
  canMsg[6] = VSI >> 8;
  
  CAN.sendMsgBuf(CAN_Air_Msg_ID, 0, 7, canMsg); 

  CAN_Air_Timestamp = millis();
}

// Send AoA data
if (millis() > CAN_AoA_Timestamp + CAN_AoA_Period + random(0, 50)) {

  canMsg[0] = AoA;
  canMsg[1] = AoA >> 8;

  CAN.sendMsgBuf(CAN_AoA_Msg_ID, 0, 2, canMsg); 

  CAN_AoA_Timestamp = millis();
 
}

// Send OAT and Humidity data
if (millis() > CAN_OAT_Timestamp + CAN_OAT_Period + random(0, 50)) {
  
  Get_OAT();
  
  canMsg[0] = OAT;
  canMsg[1] = OAT >> 8;

  canMsg[2] = Humidity;

  CAN.sendMsgBuf(CAN_OAT_Msg_ID, 0, 3, canMsg); 

  CAN_OAT_Timestamp = millis();
 
}

// Send RAW Pressure and sensor temperature data
if (millis() > CAN_RAW_Timestamp + CAN_RAW_Period + random(0, 50)) {
  
  canMsg[0] = Pressure_Int;
  canMsg[1] = Pressure_Int >> 8;

  temperature_raw = (temperature_data*200.0/2047.0)-50;

  if (temperature_raw <0) {
     canMsg[2] = 128 - temperature_raw;
  } else {
     canMsg[2] = temperature_raw;
  }
//  Serial.print("RAW pressure = ");
//  Serial.println(Pressure_Int);
//  Serial.print("Sensor temperature = ");
//  Serial.println(temperature_raw);
  
  CAN.sendMsgBuf(CAN_RAW_Msg_ID, 0, 3, canMsg); 

  CAN_RAW_Timestamp = millis();
 
}

// QNH is stored in this module as well as Display modules where it can be changed.
// Latest change in the Display module overwrites the QNH value on all devices.
// The QNH gets re-broadcasted in case one of the units goes down 
if (millis() > CAN_QNH_Timestamp + CAN_QNH_Period) {

 Send_QNH();
 CAN_QNH_Timestamp = millis();

}


 

 
}
