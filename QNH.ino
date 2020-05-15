void Send_QNH() {
  buf[0] = QNH;
  buf[1] = QNH >> 8;
  CAN.sendMsgBuf(CAN_QNH_Msg_ID, 0, 2, buf); 
}

void Write_QNH() {
    EEPROM.put(QNH_MemOffset, QNH);

}

unsigned int Read_QNH() {
  unsigned int temp_QNH = 1013;

 EEPROM.get(QNH_MemOffset, temp_QNH);

if (temp_QNH >2000 or temp_QNH < 10) {
  temp_QNH = 1013;
}
  return temp_QNH;
}
