#include <Windows.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <conio.h>
#include <ctype.h>  // Include the <ctype.h> header for the toupper() function
#include <malloc.h>
#include <string.h>
#include <math.h>

//***  SOFTWARE PROPERTY OF OHIO UNIVERSITY - AVIONICS DEPARTMENT  ***//
//***  ANDRE KALINICHENKO - SUMMER 2023  ***//

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 

//pin controls for GM Series 349 and 349H
double pins[15] = {
    0.06,   // 1 
    0.13,   // 2
    0.0,    // 3  ANALOG INPUT / STROBE LATCH
    0.0,    // 4  GND
    0.25,   // 5
    0.5,    // 6
    1.0,    // 7
    2.0,    // 8
    4.0,    // 9
    8.0,    // 10 
    16.0,   // 11 
    32.0,   // 12 
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


HANDLE hDevice;  // Handle to the USB device
char *writeData;  // Data to be written
DWORD bytesWritten, bytesRead;  // Variable to store the number of bytes written
bool run = 1;
DCB dcbSerialParams;

// Signal handler function
void handleCtrlC(int signal) {
    printf("\nCtrl+C received. Exiting program...\n");
	char temp[2] = { 0b11111111, 0b11111111};
	if(!WriteFile(hDevice, temp, 2, &bytesWritten, NULL)) {
		printf("\nAlert, Failed to stop attenuator(s)!\n", GetLastError());
			CloseHandle(hDevice);
		}else{
			CloseHandle(hDevice);
			printf("\nStop command sent to attenuator\n");
			printf("\nbye\n");
		}
    exit(0);
}

void divideUp(char writeData[]);
void prompt1(char writeData[]);

int main(){

    // Register the signal handler for SIGINT (Ctrl+C)
    signal(SIGINT, handleCtrlC);

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
	printf("Welcome to the GM 349 attenuator control program for Windows.\n\n");
	printf("Enter q to quit the program.\n");
	printf("Please select number of attenuators you will be using.\n");
	printf("Num of available attenuator options 1 - 4: \n");
	// numAtten = getch() 
	printf("Enter r to stop and reset the program for new attenuation inputs\n");
	printf("Enter the amount of attenuation to be produced in dB.\n");
	printf("Min: 0.03  Max: 63.97\n\n");
	
	writeData[0] = numAtten;

	prompt1(writeData);

	do {

		printf("\nwrite data 1: %d\n",writeData[1]);
		printf("\nwrite data 2: %d\n",writeData[2]);
		divideUp(writeData);
		printf("Leading text "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(writeData[0]));
		printf("\n");
		printf("Leading text "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(writeData[1]));

		printf("\nwrite data 1: %d\n",writeData[0]);
		printf("\nwrite data 2: %d\n",writeData[1]);


		if(!WriteFile(hDevice, writeData, 2, &bytesWritten, NULL)) {
			printf("Failed to write to the USB device. Error code: %lu\n", GetLastError());
			CloseHandle(hDevice);
			return 1;
		}else{
			printf("Wrote 2 bytes to controller succesfully, enter r to stop and reset or q to quit\n",bytesWritten);
		}
		bool stop = 1;
		while (stop){
			printf("\nWaiting for return... \n");
			printf("Arduino may need to be rest if this takes too long. \n");
        	if (ReadFile(hDevice, buffer, sizeof(buffer) - 1, &bytesRead, NULL)){
            	if (bytesRead > 0){
                	buffer[bytesRead] = '\0';
					for(int i = 0; i < bytesRead; i++)
                		printf("Received data: %d\n", buffer[i]);
					stop = 0;
            	}
        	}
    	}
		printf("\n\nEnter r for new attenuation level, s to stop attenuator, or q to quit the program");
		tempChar[0] = getch();
		if( tempChar[0] == 'q' || tempChar[0] == 'Q'){
			run = 0;
			CloseHandle(hDevice);
			char s[2] = { 0b11111111, 0b11111111};
			if(!WriteFile(hDevice, s, 2, &bytesWritten, NULL)) {
				printf("\nAlert, Failed to stop attenuator(s)!\n", GetLastError());
				CloseHandle(hDevice);
				exit(0);
			}else{
				printf("\nStop command sent to attenuator\n");
				printf("\nbye\n");
				exit(0);
			}
		}
		if(tempChar[0] == 'r'|| tempChar[0] == 'R')
			prompt1(writeData);

	}while(run);
	//printf("Data written to the USB device: %s\n", writeData);

	return 0;
}


void prompt1(char writeData[]){
	bool run = 1;
	do {
		printf("\nEnter 0 to stop\n");
		printf("Enter: ");
		double temp = 0.0;
		scanf(" %lf", &temp);// Add a space before %lf to skip leading whitespace
		//input[strlen(input)] = '\0';

		// Clear the input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

		if(temp == 0.0){
			char s[2] = { 0b11111111, 0b11111111};
			if(!WriteFile(hDevice, s, 2, &bytesWritten, NULL)) {
				printf("\nAlert, Failed to stop attenuator(s)!\n", GetLastError());
				CloseHandle(hDevice);
				exit(0);
			}else{
				printf("\nStop command sent to attenuator\n");
				printf("\n  bye\n");
				CloseHandle(hDevice);
				exit(0);
			}
		}

		if(temp < 0.03 || temp > 63.97){
			printf("\nError, input %lf is out of range\n", temp);
			printf("Please try again\n\n");
		}else{
			writeData[1] = (char)((int)floor(temp));
			temp = (temp - floor(temp)) + 0.0000001;
			//printf("HELLO: %lf", temp);
			temp *= 100;
			writeData[2] = (char)temp;
			run = 0;
		}
	}while(run);
}


void divideUp(char writeData[]){
		//char transfer1, transfer2 = (char)0;
		int lvldB2 = (int)writeData[2];
		int lvldB1 = (int)writeData[1];
		writeData[2] = 0b00000000;
		writeData[1] = 0b00000000;
		writeData[0] = 0b00000000;
		
		double temp = 0.0;

		for(int i = 14; i >= 0 ; i--){
			
			if(lvldB2 == 0)
				break;

			temp = pins[i]*100.0;
			//printf("lvldB2: %d\n", lvldB2);
			//printf("temp: %d\n", temp);
			 
			if( lvldB2 == (int)(temp) ){
				printf("hello! 1 ");
				if(i <= 7){
					writeData[0] = lookUp[i] | writeData[0];
					break;
				}else{
					printf("look up I: %d\n", lookUp[i]);
					printf("write data: %d\n", writeData[1]);

					writeData[1] = lookUp[i] | writeData[1];
					printf("\nI: %d\n", i);
					printf("hello!: %d\n", writeData[1]);
					break;
				}
			}

			if(lvldB2 <= 3)
				writeData[1] = writeData[1] | lookUp[14];

			if(pins[i] < 1.0 && pins[i] != 0){
				if(lvldB2 % (int)(pins[i]*100 + 0.0000001) != 0 && lvldB2 % (int)(pins[i]*100 + 0.0000001) != lvldB2){
					switch(i){
						case 0:
							writeData[0] = writeData[0] | lookUp[i];
							lvldB2 -= (int)(pins[i]*100 + 0.0000001);
							break;
						case 1:
							writeData[0] = writeData[0] | lookUp[i];
							lvldB2 -= (int)(pins[i]*100 + 0.0000001);
							break;
						case 4:
							writeData[0] = writeData[0] | lookUp[i];
							lvldB2 -= (int)(pins[i]*100 + 0.0000001);
							break;
						case 5:
							writeData[0] = writeData[0] | lookUp[i];
							lvldB2 -= (int)(pins[i]*100 + 0.0000001);
							break;

						default:
							break;
					}
				}
			}
		}

		if(lvldB2 <= 5)
			writeData[0] = writeData[0] | writeData[14];

		for(int i = 14; i >= 0 ; i--){
			
			if(lvldB1 == 0)
				return;

			if(lvldB1 == (int)pins[i]){
				if(i <= 7){
					writeData[0] = lookUp[i] | writeData[0];
					return;
				}else{
					writeData[1] = lookUp[i] | writeData[1];
					return;
				}
			}

			if(pins[i] >= 1.0){
				if(lvldB1 % (int)pins[i] != 0 && lvldB1 % (int)pins[i] != lvldB1){
					switch(i){
						case 6:
							writeData[0] = writeData[0] | lookUp[i];
							lvldB1 -= (int)pins[i];
							break;
						case 7:
							writeData[0] = writeData[0] | lookUp[i];
							lvldB1 -= (int)pins[i];
							break;
						case 8:
							writeData[1] = writeData[1] | lookUp[i];
							lvldB1 -= (int)pins[i];
							printf("\n4: %d\n", writeData[1]);
							break;
						case 9:
							writeData[1] = writeData[1] | lookUp[i];
							lvldB1 -= (int)pins[i];
							printf("\n8: %d\n", writeData[1]);
							break;
						case 10:
							writeData[1] = writeData[1] | lookUp[i];
							lvldB1 -= (int)pins[i];
							printf("\n16: %d\n", writeData[1]);
							break;
						case 11:
							writeData[1] = writeData[1] | lookUp[i];
							lvldB1 -= (int)pins[i];
							printf("\n32: %d\n", writeData[1]);
							break;
					default:
						break;
					}
				}
				//printf("MOD: %d\n",lvldB);
			}
		}
}