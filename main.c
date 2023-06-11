#include <Windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <conio.h>
#include <malloc.h>
#include <string.h>
#include <math.h>

//pin controls for GM Series 349 and 349H
double pins[15] = {
    0.06,   // 1 
    0.13,   // 2
    0.0,    // 3  ANALOG INPUT / STROBE LATCH
    0.0,    // 4  GND
    0.25,   // 5
    0.5,    // 6
    1.0,    // 7  //57
    2.0,    // 8
    4.0,    // 9
    8.0,    // 10 //57
    16.0,   // 11 //57
    32.0,   // 12 //57
    0.0,    // 13  +12V TO +15V
    0.0,    // 14  -12V TO -15V
    0.03    // 15
};

//  last 2 bits are used to set number of attenuators used 1 through 4  ( [0,1,2,3] )
//  bit set 1 indicates that pin will be set on, bit set 0 indicates pin that pin will be set off
char lookUp[15] = {
    0b10000000, // 0.06
    0b01000000, // 0.13
    0b00100000, // 0.0   // ANALOG INPUT / STROBE LATCH
    0b00010000, // 0.0   // GND
    0b00001000, // 0.25
    0b00000100, // 0.5
    0b00000010, // 1.0
    0b00000001, // 2.0
    0b10000000, // 4.0
    0b01000000, // 8.0
    0b00100000, // 16.0
    0b00010000, // 32.0 
    0b00001000, // 0.0   // +12V TO +15V
    0b00000100, // 0.0   // -12V TO -15V
    0b00000100  // 0.03
};

//  00000010 01110000

HANDLE hDevice;  // Handle to the USB device
char *writeData;  // Data to be written
DWORD bytesWritten, bytesRead;  // Variable to store the number of bytes written
bool run = 1;
DCB dcbSerialParams;

void divideUp(char writeData[]);

int main(){

	writeData = (char*)malloc(5);

	char numAtten = 'H';

	int lvldB = 0.0;
	char buffer[256];
	char tempChar[10];

	hDevice = CreateFile(
		"COM5",  // Replace COMx with the appropriate USB port identifier (e.g., "COM1" or "COM2")
		(GENERIC_WRITE | GENERIC_READ),
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hDevice == INVALID_HANDLE_VALUE) {
		printf("Failed to open the USB device. Error code: %lu\n", GetLastError());
		return 1;
	}

	dcbSerialParams.BaudRate = CBR_9600; // Replace with your baud rate
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if (!SetCommState(hDevice, &dcbSerialParams)){
        printf("Error setting serial port state.\n");
        CloseHandle(hDevice);
        return 1;
    }

	printf("\n\n");
	printf("Hello.\n");
	printf("Welcome to the windows GM 349 attenuator control program.\n\n");
	printf("Enter q to quit the program.\n");
	printf("Please select number of attenuators you will be using.\n");
	printf("Num of available attenuator options 1 - 4: \n");
	// numAtten = getch() 
	printf("Enter r to stop and reset the program for new attenuation inputs\n");
	printf("Enter the amount of attenuation to be produced in dB.\n");
	printf("Min: 0.06  Max: 63.97\n\n");
	
	writeData[0] = numAtten;
	
	printf("Enter: ");
	double temp =0.0;
	scanf("%lf", &temp);
	writeData[1] = (char)((int)floor(temp));
	temp = temp - floor(temp);
	//printf("HELLO: %lf", temp);
	temp *= 100;
	writeData[2] = (char)temp;

	// printf("\n");
	// printf("write data: %d", writeData[0]);
	// printf("\n");
	// printf("write data: %d", writeData[1]);
	// printf("\n");
	// printf("write data: %d", writeData[2]);
	// printf("\n");
	
	do {
		divideUp(writeData);
		printf("WRITE DATA: %d\n", writeData[0]);
		printf("WRITE DATA: %d\n", writeData[1]);

		if(!WriteFile(hDevice, writeData, 2, &bytesWritten, NULL)) {
			printf("Failed to write to the USB device. Error code: %lu\n", GetLastError());
			CloseHandle(hDevice);
			return 1;
		}else{
			printf("Wrote 2 bytes to controller succesfully, enter r to stop and reset or q to quit\n",bytesWritten);
		}
		while (1){
			printf("Waiting for return ...\n");
        	if (ReadFile(hDevice, buffer, sizeof(buffer) - 1, &bytesRead, NULL)){
            	if (bytesRead > 0){
                	buffer[bytesRead] = '\0';

					for(int i = 0; i < bytesRead; i++)
                		printf("Received data: %d\n", buffer[i]);

					break;
            	}
        	}
    	}
		tempChar[0] = getch();
		if( tempChar[0] == 'q' || tempChar[0] == 'Q'){
			run = 0;
		}
		if(tempChar[0] == 'r');
	}while(run);
	//printf("Data written to the USB device: %s\n", writeData);
}


void divideUp(char writeData[]){

		char transfer1, transfer2 = (char)0;
		int lvldB2 = (int)writeData[2];
		int lvldB1 = (int)writeData[1];
		double temp = 0.0;

		for(int i = 14; i >= 0 ; i--){
			
			if(lvldB2 == 0)
				break;
			temp = pins[i]*100.0;

			if( lvldB2 == (int)(temp) ){
				//printf("hello!");
				if(i <= 7){
					writeData[0] = lookUp[i]  | transfer1;
					writeData[1] = transfer2;
					break;
				}else{
					writeData[0] = transfer1;
					writeData[1] = lookUp[i]  | transfer2;
					printf("hello!: %d\n", writeData[1]);
					break;
				}
			}

			if(pins[i] < 1.0 && pins[i] != 0){
				if(lvldB2 % (int)(pins[i]*100) != 0 && lvldB2 % (int)(pins[i]*100) != lvldB2){
					printf("hello!");
					switch(i+1){
						case 1:
							transfer1 = transfer1 | 0b10000000;
							lvldB2 -= (int)pins[i]*100;
							break;
						case 2:
							transfer1 = transfer1 | 0b01000000;
							lvldB2 -= (int)pins[i]*100;
							break;
						case 5:
							transfer1 = transfer1 | 0b00001000;
							lvldB2 -= (int)pins[i]*100;
							break;
						case 6:
							transfer1 = transfer1 | 0b00000100;
							lvldB2 -= (int)pins[i]*100;
							break;
						case 15:
							transfer2 = transfer2 | 0b00000100;
							lvldB2 -= (int)pins[i]*100;
							break;
						default:
							break;
					}
				}
			}
		}

		for(int i = 14; i >= 0 ; i--){
			
			if(lvldB1 == 0)
				return;

			if(lvldB1 == (int)pins[i]){
				if(i <= 7){
					writeData[0] = lookUp[i]  | transfer1;
					//writeData[1] = transfer2;
					return;
				}else{
					//writeData[0] = transfer1;
					writeData[1] = lookUp[i]  | transfer2;
					return;
				}
			}

			if(pins[i] >= 1.0){
				if(lvldB1 % (int)pins[i] != 0 && lvldB1 % (int)pins[i] != lvldB1){
					switch(i+1){
						case 7:
							transfer1 = transfer1 | 0b00000010;
							lvldB1 -= (int)pins[i];
							break;
						case 8:
							transfer1 = transfer1 | 0b00000001;
							lvldB1 -= (int)pins[i];
							break;
						case 9:
							transfer2 = transfer2 | 0b10000000;
							lvldB1 -= (int)pins[i];
							break;
						case 10:
							transfer2 = transfer2 | 0b01000000;
							lvldB1-= (int)pins[i];
							break;
						case 11:
							transfer2 = transfer2 | 0b00100000;
							lvldB1 -= (int)pins[i];
							break;
						case 12:
							transfer2 = transfer2 | 0b00010000;
							lvldB1 -= (int)pins[i];
							break;
					default:
						break;
					}
				}
				//printf("MOD: %d\n",lvldB);
			}
		}
}