#include <Windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <conio.h>
#include <malloc.h>
#include <string.h>
#include <math.h>

//pin controls for GM Series 349 and 349H
double pins[15] = {
        0.06,  // 1 
        0.13,  // 2
         0.0,  // 3  ANALOG INPUT / STROBE LATCH
         0.0,  // 4  GND
        0.25,  // 5
        0.5,   // 6
        1.0,   // 7
        2.0,   // 8
        4.0,   // 9
        8.0,   // 10
        16.0,  // 11
        32.0,  // 12
        0.0,   // 13  +12V TO +15V
        0.0,   // 14  -12V TO -15V
        0.03   // 15
    };

HANDLE hDevice;  // Handle to the USB device
char *writeData;  // Data to be written
DWORD bytesWritten, bytesRead;  // Variable to store the number of bytes written
bool run = 1;
DCB dcbSerialParams;

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
	if(temp >= 10.0){
		writeData[1] = (char)((int)floor(temp));
		temp = temp - floor(temp);
		temp *= 100;
		writeData[2] = (char)temp;
	}else{
		writeData[1] = (char)((int)temp);
	}
	char transfer1, transfer2 = (char)0;
	if(writeData[2] == 0){
		lvldB = (int)writeData[1];
		for(int i = 14; i >= 0 ; i--){
			if(pins[i] != 0.0 && (int)pins[i] != 0.0){
				if(lvldB % (int)pins[i] == 0){
					switch(i+1){
						case 7:
							transfer1 = transfer1 | 0b00000010;
							break;
						case 8:
							transfer1 = transfer1 | 0b00000001;
							break;
						case 9:
							transfer2 = transfer2 | 0b10000000;
							break;
						case 10:
							transfer2 = transfer2 | 0b0100000;
							break;
						case 11:
							transfer2 = transfer2 | 0b00100000;
							break;
						case 12:
							transfer2 = transfer2 | 0b00010000;
							break;
					break;
					}
					lvldB -= (int)pins[i];
				}
				printf("MOD: %d\n",lvldB);
			}
		}
	}
	
	do {
		if(!WriteFile(hDevice, writeData, strlen(writeData), &bytesWritten, NULL)) {
			printf("Failed to write to the USB device. Error code: %lu\n", GetLastError());
			CloseHandle(hDevice);
			return 1;
		}else{
			printf("Wrote %d bytes to controller succesfully, enter r to stop and reset or q to quit\n",bytesWritten);
		}
		while (1){
			printf("Waiting for return ...\n");
        	if (ReadFile(hDevice, buffer, sizeof(buffer) - 1, &bytesRead, NULL)){
            	if (bytesRead > 0){
                	buffer[bytesRead] = '\0';
                	printf("Received data: %s\n", buffer);
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