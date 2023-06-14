int incomingByte = 0;   // for incoming serial data
int i = 0;
int instr[2] = { 0, 0 };
int pinArray[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int pinArray2[11] = { 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 };

void setup() {
  // put your setup code here, to run once:
    pinMode(22, OUTPUT); //1
    pinMode(23, OUTPUT); //2
    pinMode(24, OUTPUT); //3
    pinMode(25, OUTPUT); //4
    pinMode(26, OUTPUT); //5
    pinMode(27, OUTPUT); //6 
    pinMode(28, OUTPUT); //7
    pinMode(29, OUTPUT); //8
    pinMode(30, OUTPUT); //9
    pinMode(31, OUTPUT); //10
    pinMode(32, OUTPUT); //11
    
    Serial.begin(9600);     // opens serial port, sets data rate to 9600 bps
}

void loop() {
  // put your main code here, to run repeatedly:

  if (Serial.available() == 2) {
    
    //incomingByte = Serial.read();
    instr[0] = Serial.read();
    instr[1] = Serial.read();
    //incomingByte = Serial.read();
    //instr[1] = incomingByte;
    //Serial.print("received: ");
    //Serial.println(instr[0]);
    //Serial.println(instr[1]);
    // say what you got:
    //Serial.print("received: ");
  }

  for(int i = 7; i >= 0; i--){
      if(instr[0] & (1 << i))
        switch(i){
          case 7:
              pinArray[0] = 22;
            break;
          case 6:
              pinArray[1] = 23;
            break;
          case 3:
              pinArray[2] = 24;
          break;
          case 2:
              pinArray[3] = 25;
          break;
          case 1:
              pinArray[4] = 26;
          break;
          case 0:
              pinArray[5] = 27;
          break;
        }
  }
  for(int i = 7; i >= 0; i--){
      if(instr[1] & (1 << i))
        switch(i){
          case 7:
              pinArray[6] = 28;
            break;
          case 6:
              pinArray[7] = 29;
            break;
          case 5:
              pinArray[8] = 30;
          break;
          case 4:
              pinArray[9] = 31;
          break;
          case 2:
              pinArray[10] = 32;
          break;
        }
  }

  for(int i = 0; i < 11; i++){
      digitalWrite(pinArray[i], HIGH);
      // delay(50);
      // digitalWrite(pinArray[i], LOW);
      // delay(50);
  }
  
  // if(instr[0] == 0b11001111){
  //       digitalWrite(22, HIGH);
  //       delay(500);
  //       digitalWrite(22, LOW);
  //       delay(500);
  //       digitalWrite(22, HIGH);
  //       delay(500);
  //       digitalWrite(22, LOW);
  //       delay(500);
  // }
  // if(instr[1] == 0b11110000 ){
  //       digitalWrite(23, HIGH);
  //       delay(500);
  //       digitalWrite(23, LOW);
  //       delay(500);
  //       digitalWrite(23, HIGH);
  //       delay(500);
  //       digitalWrite(23, LOW);
  //       delay(500);
  // }

  // int temp = 0b01110000;
  // Serial.println(" ");
  // Serial.println((temp & (1 << 7)),BIN);

  // if(instr[0] != 0 || instr[1] != 0){
  //   //check if the 8th bit is set to 1
  //   if (instr[0] & (1 << 7))  {
  //       digitalWrite(22, HIGH);
  //       delay(100);
  //       digitalWrite(22, LOW);
  //       delay(100);
  //       digitalWrite(22, HIGH);
  //       delay(100);
  //       digitalWrite(22, LOW);
  //       delay(100);
  //   }
  //   if (instr[0] & (1 << 6)) {
  //       digitalWrite(23, HIGH);
  //       delay(100);
  //       digitalWrite(23, LOW);
  //       delay(100);
  //       digitalWrite(23, HIGH);
  //       delay(100);
  //       digitalWrite(23, LOW);
  //       delay(100);
  //   }
  //   if (instr[0] & (1 << 5)) {
  //       digitalWrite(24, HIGH);
  //       delay(100);
  //       digitalWrite(24, LOW);
  //       delay(100);
  //       digitalWrite(24, HIGH);
  //       delay(100);
  //       digitalWrite(24, LOW);
  //       delay(100);
  //   }
  //   if (instr[0] & (1 << 4)) {
  //       digitalWrite(25, HIGH);
  //       delay(100);
  //       digitalWrite(25, LOW);
  //       delay(100);
  //       digitalWrite(25, HIGH);
  //       delay(100);
  //       digitalWrite(25, LOW);
  //       delay(100);
  //   }
  //   if (instr[0] & (1 << 3)) {
  //       digitalWrite(26, HIGH);
  //       delay(100);
  //       digitalWrite(26, LOW);
  //       delay(100);
  //       digitalWrite(26, HIGH);
  //       delay(100);
  //       digitalWrite(26, LOW);
  //       delay(100);
  //   }
  //   if (instr[0] & (1 << 2)) {
  //       digitalWrite(27, HIGH);
  //       delay(100);
  //       digitalWrite(27, LOW);
  //       delay(100);
  //       digitalWrite(27, HIGH);
  //       delay(100);
  //       digitalWrite(27, LOW);
  //       delay(100);
  //   }
  //   if (instr[0] & (1)) {
  //       digitalWrite(28, HIGH);
  //       delay(100);
  //       digitalWrite(28, LOW);
  //       delay(100);
  //       digitalWrite(28, HIGH);
  //       delay(100);
  //       digitalWrite(28, LOW);
  //       delay(100);
  //   }
  //   if (instr[1] & (1 << 7)) {
  //       digitalWrite(29, HIGH);
  //       delay(100);
  //       digitalWrite(29, LOW);
  //       delay(100);
  //       digitalWrite(29, HIGH);
  //       delay(100);
  //       digitalWrite(29, LOW);
  //       delay(100);
  //   }
  //   if (instr[1] & (1 << 6)) {
  //       digitalWrite(30, HIGH);
  //       delay(100);
  //       digitalWrite(30, LOW);
  //       delay(100);
  //       digitalWrite(30, HIGH);
  //       delay(100);
  //       digitalWrite(30, LOW);
  //       delay(100);
  //   }
  //   if (instr[1] & (1 << 5)) {
  //       digitalWrite(31, HIGH);
  //       delay(100);
  //       digitalWrite(31, LOW);
  //       delay(100);
  //       digitalWrite(31, HIGH);
  //       delay(100);
  //       digitalWrite(31, LOW);
  //       delay(100);
  //   }
  //   if (instr[1] & (1 << 4)) {
  //       digitalWrite(32, HIGH);
  //       delay(100);
  //       digitalWrite(32, LOW);
  //       delay(100);
  //       digitalWrite(32, HIGH);
  //       delay(100);
  //       digitalWrite(32, LOW);
  //       delay(100);
  //   }
  // }
}