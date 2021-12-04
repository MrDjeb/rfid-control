void Read(byte *data, byte block) {
  byte buff[18];
  byte buffSize = sizeof(buff);
  MFRC522::StatusCode stat;
  stat = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (stat != MFRC522::STATUS_OK) {Serial.print("Authenticate failed to block ");Serial.print(block);Serial.print(" becouse: ");Serial.println(mfrc522.GetStatusCodeName(stat)); return;}
  stat = mfrc522.MIFARE_Read(block, buff, &buffSize);
  if (stat != MFRC522::STATUS_OK) {Serial.print("Reading failed to block ");Serial.print(block);Serial.print(" becouse: ");Serial.println(mfrc522.GetStatusCodeName(stat)); return;}
  for (int i = 0; i < 16; i++) data[i] = buff[i];
}

void Write(String data, byte block) {
  byte buff[16];
  data.getBytes(buff, sizeof(buff));
  buff[15]=char(data[15]);
  MFRC522::StatusCode stat;
  stat = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (stat != MFRC522::STATUS_OK) {Serial.print("Authenticate failed to block ");Serial.print(block);Serial.print(" becouse: ");Serial.println(mfrc522.GetStatusCodeName(stat)); return;}
  stat = mfrc522.MIFARE_Write(block, buff, sizeof(buff));
  if (stat != MFRC522::STATUS_OK) {Serial.print("Writing failed to block ");Serial.print(block);Serial.print(" becouse: ");Serial.println(mfrc522.GetStatusCodeName(stat)); return;}
}

String Read_hash() {
  byte byte1[17], byte2[17];
  Read(&byte1[0], BLOCK1);
  Read(&byte2[0], BLOCK2);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  byte1[16]=0;byte2[16]=0;
  return String((char*)byte1)+String((char*)byte2);
}

void Write_hash(String data) {
  Serial.print("Writing new hash on card: "); Serial.println(data); 
  Write(data.substring(0,16), BLOCK1);
  Write(data.substring(16,32), BLOCK2);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  Serial.println();
}
