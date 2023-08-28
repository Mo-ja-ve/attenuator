int incomingByte = 0;   // for incoming serial data
int i = 0;
int instr[2] = { 0, 0 };
int pinArray[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int pinArray2[11] = { 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 };

//String sPins = "";

void setup() {
  // put your setup code here, to run once:
    pinMode(13, OUTPUT); 
    pinMode(12, OUTPUT); 
    pinMode(11, OUTPUT); 
    pinMode(10, OUTPUT);

    pinMode(22, OUTPUT); 
    pinMode(23, OUTPUT); 
    pinMode(24, OUTPUT); 
    pinMode(25, OUTPUT); 
    pinMode(26, OUTPUT); 
    pinMode(27, OUTPUT);  
    pinMode(28, OUTPUT); 
    pinMode(29, OUTPUT); 
    pinMode(30, OUTPUT); 
    pinMode(31, OUTPUT); 
    pinMode(32, OUTPUT); 
    pinMode(33, OUTPUT);
    pinMode(34, OUTPUT); 
    pinMode(35, OUTPUT); 
    pinMode(36, OUTPUT); 
    pinMode(37, OUTPUT); 
    pinMode(38, OUTPUT); 
    pinMode(39, OUTPUT);  
    pinMode(40, OUTPUT); 
    pinMode(41, OUTPUT); 
    pinMode(42, OUTPUT); 
    pinMode(43, OUTPUT);
    pinMode(44, OUTPUT);
    pinMode(45, OUTPUT);
    pinMode(46, OUTPUT);
    pinMode(47, OUTPUT);
    pinMode(48, OUTPUT);
    pinMode(49, OUTPUT);
    pinMode(50, OUTPUT);
    pinMode(51, OUTPUT);
    pinMode(52, OUTPUT);
    pinMode(53, OUTPUT);

    PORTC = 0b00000000;
    PORTB = 0b00000000;
    PORTL = 0b00000000;
    PORTA = 0b00000000;
    
    Serial.begin(9600);     // opens serial port, sets data rate to 9600 bps
}

void loop() {
  // put your main code here, to run repeatedly:
  String sPins = "";
  //PORTA = 0b00010000;
   //PORTC = 0b00000000;
  //PORTB = 0b00100100;
  
  //Serial.print(PORTA);
  //digitalWrite(25, LOW);
  // digitalWrite(50, LOW);
  // digitalWrite(51, LOW);
  // digitalWrite(52, HIGH);
  // digitalWrite(53, HIGH);

  //command to set arduino to run in 'program read from file mode'
  if(Serial.read() == 'a'){
    //PORTA = 0b00000001;

    char temp;

    int stop = 1;
    while(stop)
      while( Serial.available() > 0){
        temp = Serial.read();
        // if(temp == '0b01111010'){
        //   temp = Serial.read();
        // }else 
        if(temp == '\0'){
          stop = 0;
        }else{
          sPins += temp;
        }
    }

    //Serial.print(sPins[i]);
    // Serial.print((sPins[i] >> 6),BIN);
    
    for(int i = 0; i < sPins.length(); i++){
      if((sPins[i] >> 6) == 0)
        PORTA = sPins[i]; 

      if((sPins[i] >> 6) == 1)
        PORTC = sPins[i];

      if((sPins[i] >> 6) == -2)
        PORTL = sPins[i];

      if((sPins[i] >> 6) == -1)
        PORTB = sPins[i];
    }

  }else{

    if(Serial.read() == 'b')
      if(Serial.read() == '\0'){
        PORTC = 0b00000000;
        PORTB = 0b00000000;
        PORTL = 0b00000000;
        PORTA = 0b00000000;
      }
  }
}