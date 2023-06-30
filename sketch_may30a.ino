int incomingByte = 0;   // for incoming serial data
int i = 0;
int instr[2] = { 0, 0 };
int pinArray[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int pinArray2[11] = { 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 };
String sPins = "";

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

  //command to set arduino to run in 'program read from file mode'
  if(Serial.read() == 'a'){
        // for(int i = 0; i < 11; i++){
        //   digitalWrite(pinArray2[i], HIGH);
        // }
    char temp;
    //Serial.write('z');
    //Serial.flush();
    int stop = 1;
    while(stop)
      while( Serial.available() > 0){
        temp = Serial.read();
        //Serial.write(temp);
        if(temp != '\0'){
          sPins += temp;
        }else{
          stop = 0;
        }
      }
    
    for(int i = 0; i < sPins.length(); i++)
      for(int j = 7; j >= 0; j--)
        (sPins[i] & (1 << j)) ? digitalWrite(27-j, HIGH) : digitalWrite(27-j, LOW);

          // switch(j){
          //   case 7:
          //       pinArray[0] = 0;
          //     break;
          //   case 6:
          //       pinArray[1] = 0;
          //     break;
          //   case 5:
          //       digitalWrite(22, HIGH);// 1dB
          //     break;
          //   case 4:
          //       digitalWrite(23, HIGH);// 2dB
          //     break;
          //   case 3:
          //       digitalWrite(24, HIGH);// 4dB
          //     break;
          //   case 2:
          //       digitalWrite(25, HIGH);// 8dB
          //     break;
          //   case 1:
          //       digitalWrite(26, HIGH);// 16dB
          //     break;
          //   case 0:
          //       digitalWrite(27, HIGH);// 32dB
          //     break;
          // } :
          // switch(j){
          //   case 7:
          //       pinArray[0] = 0;
          //     break;
          //   case 6:
          //       pinArray[1] = 0;
          //     break;
          //   case 5:
          //       digitalWrite(22, LOW;// 1dB
          //     break;
          //   case 4:
          //       digitalWrite(23, LOW);// 2dB
          //     break;
          //   case 3:
          //       digitalWrite(24, LOW);// 4dB
          //     break;
          //   case 2:
          //       digitalWrite(25, LOW);// 8dB
          //     break;
          //   case 1:
          //       digitalWrite(26, LOW);// 16dB
          //     break;
          //   case 0:
          //       digitalWrite(27, LOW);// 32dB
          //     break;
          // };

  }else
  if (Serial.available() == 2) {
    
    Serial.flush();
    instr[0] = Serial.read();
    instr[1] = Serial.read();
    
    char temp1 = (char)instr[0];
    char temp2 = (char)instr[1];
    
    Serial.print(temp1);
    Serial.print(temp2);
    //Serial.print(instr[1],DEC);
  
  if(instr[0] == 0b11111111 && instr[1] == 0b11111111){
    instr[0] = 0b00000000;
    instr[1] = 0b00000000;
    for(int i = 0; i < 52; i++){
      digitalWrite(i, LOW);
    }
    for(int i = 0; i < 11; i++){
      pinArray[i] = 0;
    }
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
    }
  }
}