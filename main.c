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
#include <time.h>


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
	//pins 1 through 6 will are no longer used due to program updates requested by Dr Bartone
    0.06,   // 1 
    0.13,   // 2
    0.0,    // 3  ANALOG INPUT / STROBE LATCH
    0.0,    // 4  GND
    0.25,   // 5
    0.5,    // 6
	//pins 1 through 6 will are no longer used due to program updates requested by Dr Bartone
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
	//pins 1 through 6 will are no longer used due to program updates requested by Dr Bartone
    0b10000000, // 0.06
    0b01000000, // 0.13
    0b00100000, // 0.0   // ANALOG INPUT / STROBE LATCH
    0b00010000, // 0.0   // GND
    0b00001000, // 0.25
    0b00000100, // 0.5
	//pins 1 through 6 will are no longer used due to program updates requested by Dr Bartone
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
int **intInstr;
int INSTR_LENGTH = 0;

// Signal handler function
void handleCtrlC(int signal) {
    printf("\nCtrl+C received. Exiting program...\n");
	char temp[2] = { 0b11111111, 0b11111111};
	if(!WriteFile(hDevice, temp, 2, &bytesWritten, NULL)) {
		printf("\n\nAlert, Failed to stop attenuator(s)!(they may still be running).\n", GetLastError());
			CloseHandle(hDevice);
			    exit(0);
		}else{
			CloseHandle(hDevice);
			printf("\n\nStop command sent to attenuator\n");
			printf("\nbye\n");
			    exit(0);
		}
    exit(0);
}

void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;
 
    // Storing start time
    clock_t start_time = clock();
 
    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}

void divideUp(char writeData[]);
void prompt1(char writeData[]);
int convertToInteger(char* numStr);
int launchInstr(int **intInstr);
char setPins(int lvldB);

int main(int argc, char* argv[]){
	FILE *file;
    char **instr;  // 2D array to store lines

	if( argc == 2) {

		printf("Hello.\n");
		printf("Welcome to the GM 349 attenuator control program for Windows.\n\n");
		printf("You have supplied command line arguments which have actived this programs 'run using input file mode'. \n");
		printf("Choosing to run this pogram with no file name supplied will instead activate the programs manual user mode, you will be prompted with further instructions once you enter that mode.\n\n");
    	printf("The file supplied is %s\n", argv[1]);
		
		file = fopen("text.txt", "r");
   		if (file == NULL) {
        	printf("Failed to open the file. are you sure you typed the files name correctly?\n");
        	return 1;
    	}
		
		char byte = 0;
		int j = 0;
		int i = 0;

		instr = malloc(sizeof(char *));
		instr[0] = malloc(sizeof(char));

		do{
			byte = fgetc(file);
			
			if(byte == EOF)
				break;

			if(byte == '\n'){
				instr[i][j] = '\0';
				i++;
				j = 0;
				instr = realloc(instr, sizeof(char*) * (i+1));
				instr[i] = malloc(sizeof(char));
			}else{
				j++;
				instr[i] = realloc(instr[i], sizeof(char) * (j+1));
				instr[i][j-1] = byte;
			}
		} while(byte != EOF);

		INSTR_LENGTH = i;
		instr = realloc(instr, sizeof(char*) * (INSTR_LENGTH));

		j=0;
		
		// for(int i = 0; i < INSTR_LENGTH; i++){
		// 	while(instr[i][j] != '\0'){
		// 		printf("%c",instr[i][j]);
		// 		j++;
		// 	}
		// 	j=0;
		// 	//printf("I: %d", i);
		// 	printf("\n");
		// }
		
		j = 0;
		char *temp;
		int result = 0;
		int k = 0;
		intInstr = malloc(sizeof(int *));
		intInstr[0] = malloc(sizeof(int ));
		
		temp = malloc(sizeof(char));

		// for(int ii = 0; ii < INSTR_LENGTH; ii++){
		// 	int jj = 0;
		// 	while(instr[ii][jj] != '\0'){
		// 		printf("%c", instr[ii][jj]);
		// 		jj++;
		// 	}
		// 	printf("\n");
		// }
		
		for(i = 0; i<INSTR_LENGTH; i++){
			int j2 = 0;
			intInstr = realloc(intInstr, sizeof(int*)*(i+1));
			intInstr[i] = malloc(sizeof(int));
			while(instr[i][j] != '\0'){
				if(instr[i][j] != ' '){
					temp = realloc(temp, sizeof(char)*(j2+1));
					temp[j2] = instr[i][j];
					j2++;
				}else{
					temp = realloc(temp, sizeof(char)*(j2+1));
					temp[j2] = '\0';
					result = convertToInteger(temp);
					intInstr[i] = realloc(intInstr[i], sizeof(int)*(k+1));
					intInstr[i][k] = result;
					k++;
					j2=0;
				}
				j++;
			}
			temp = realloc(temp, sizeof(char)*(j2+1));
			temp[j2] = '\0';
			result = convertToInteger(temp);

			intInstr[i] = realloc(intInstr[i], sizeof(int)*(k+1));
			intInstr[i][k] = result;
			intInstr[i] = realloc(intInstr[i], sizeof(int)*(k+2));
			intInstr[i][k+1] = -1;
			k =0;
			j =0;
			j2=0;
		}

		int a = 0;
		for(int z = 0; z<INSTR_LENGTH; z++){
			while(intInstr[z][a] != -1){
				printf("%d ", intInstr[z][a]);
				a++;
			}
			// if(intInstr[INSTR_LENGTH][a] == -1){
			// int j2 = 0;
			// 	while(instr[INSTR_LENGTH][j] != '\0'){
			// 		if(instr[INSTR_LENGTH][j] != ' '){
			// 			temp = realloc(temp, sizeof(char)*(j2+1));
			// 			temp[j2] = instr[INSTR_LENGTH][j];
			// 			j2++;
			// 		}else{
			// 			temp = realloc(temp, sizeof(char)*(j2+1));
			// 			temp[j2] = '\0';
			// 			result = convertToInteger(temp);
			// 			intInstr[i] = realloc(intInstr[i], sizeof(int)*(k+1));
			// 			intInstr[i][k] = result;
			// 			k++;
			// 			j2=0;
			// 		}
			// 		j++;
			// 	}
			// }
			printf("\n");
			a=0;
		}

		launchInstr(intInstr);

   } else if( argc > 2 ) {
     	printf("Too many arguments supplied.\n");
		
   }else {

		signal(SIGINT, handleCtrlC);

		writeData = (char*)malloc(5);

		char numAtten = 'H';

		int lvldB = 0.0;
		char buffer[256];
		char tempChar[10];

		printf("\n\n");
		printf("Hello.\n");
		printf("Welcome to the GM 349 attenuator control program for Windows.\n\n");
		printf("Enter q to quit the program.\n");
		printf("Please select number of attenuators you will be using.\n");
		printf("Num of available attenuator options 1 - 4: \n");
		// numAtten = getch() 
		printf("Enter r to stop and reset the program for new attenuation inputs\n");
		printf("Enter the amount of attenuation to be produced in dB.\n");
		printf("Min: 0.03  Max: 63.97\n");

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
						printf("\nBYTES SIZE: %d\n", bytesRead);
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
				char s[2] = { 0b11111111, 0b11111111};
				if(!WriteFile(hDevice, s, 2, &bytesWritten, NULL)) {
					printf("\n\nAlert, Failed to stop attenuator(s)!(they may still be running).\n\n", GetLastError());
					CloseHandle(hDevice);
					exit(0);
				}else{
					printf("\nStop command sent to attenuator\n");
					printf("\nbye\n");
					CloseHandle(hDevice);
					exit(0);
				}
			}

			if( tempChar[0] == 's' || tempChar[0] == 'S'){
				run = 0;
				char s[2] = { 0b11111111, 0b11111111};
				if(!WriteFile(hDevice, s, 2, &bytesWritten, NULL)) {
					printf("\n\nAlert, Failed to stop attenuator(s)!(they may still be running).\n\n");
					printf("Error number: %lu\n", GetLastError());
				}else{
					printf("\nStop command sent to attenuator\n");
				}
			}

			if(tempChar[0] == 'r'|| tempChar[0] == 'R'){
				char s[2] = { 0b11111111, 0b11111111};
				if(!WriteFile(hDevice, s, 2, &bytesWritten, NULL)) {
					printf("\n\nAlert, Failed to stop attenuator(s)!(they may still be running).\n\n");
					printf("Error number: %lu\n", GetLastError());
				}else{
					printf("\nStop command sent to attenuator\n");
				}
				prompt1(writeData);
			}

		}while(run);
	}

	return 0;
}


void prompt1(char writeData[]){
	bool run = 1;
	do {
		printf("\nEnter 0 to quit and stop attenuator(s).\n");
		printf("Enter: ");
		double temp = 0.0;
		scanf(" %lf", &temp);// Add a space before %lf to skip leading whitespace

		// Clear the input buffer
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

		if(temp == 0.0){
			char s[2] = { 0b11111111, 0b11111111};
			if(!WriteFile(hDevice, s, 2, &bytesWritten, NULL)) {
				printf("\n\nAlert, Failed to stop attenuator(s)!(they may still be running).\n\n");
				printf("Error number: %lu\n", GetLastError());
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
			temp *= 100;
			writeData[2] = (char)temp;
			run = 0;
		}
	}while(run);
}

int launchInstr(int **intInstr){
   // Register the signal handler for SIGINT (Ctrl+C)
   	char buffer[256];

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

	char **pinOuts;
	int j = 0;
	pinOuts = malloc(sizeof(char*));
	pinOuts[0] = malloc(sizeof(char));

	for(int i = 0; i <INSTR_LENGTH; i++){
		pinOuts = realloc(pinOuts, sizeof(char*)*(i+1));
		pinOuts[i] = malloc(sizeof(char));
		while(intInstr[i][j] != -1){
			pinOuts[i] = realloc(pinOuts[i], sizeof(char)*(j+1));
			pinOuts[i][j] = 0b00000000;
			j++;
		}
		j = 0;
	}

	j = 1;
	int lvldB = 0;
	int pinOutsWidth = 0;
	for(int i = 0; i < INSTR_LENGTH; i++){
		pinOuts[i] = realloc(pinOuts[i],sizeof(char));
	}
	j=0;
	
	// for(int i = 0; i < INSTR_LENGTH; i++){
	// 	while(pinOuts[i][j] != '\0'){
	// 		printf("  char: "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(pinOuts[i][j]));
	// 		j++;
	// 	}
	// 	printf("\n");
	// }

	for(int i = 0; i < INSTR_LENGTH; i++){
		while(intInstr[i][j] != -1){
			if(j % 2 != 0){
			lvldB = intInstr[i][j+1];
			pinOuts[i] = realloc(pinOuts[i], sizeof(char)*(pinOutsWidth+1));
			pinOuts[i][pinOutsWidth] = 0b00000000; 
			switch(intInstr[i][j]){
				case 1:
					if(lvldB > 63)
					printf("\nWarning! Argument supplied on line %d to attenuator(s) 1 is larger than 63dB!\n", i);
					//printf("char1: "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(pinOuts[i][pinOutsWidth]));
					pinOuts[i][pinOutsWidth] |= setPins(lvldB);
					pinOutsWidth++;
				break;

				case 12:
					if(lvldB > 126)
						printf("\nWarning! Argument supplied on line %d to attenuator(s) 1 2 is larger than 126dB!\n", i);

					if(lvldB <= 63){
						int temp = lvldB / 2;
						pinOuts[i][pinOutsWidth] |= setPins(temp);;;
						temp = lvldB - temp;
						pinOutsWidth++;
						pinOuts[i] = realloc(pinOuts[i], sizeof(char)*(pinOutsWidth+1));
						pinOuts[i][pinOutsWidth] =  0b01000000 | setPins(temp);
					}else{
						int temp = lvldB - 63;
						//printf("\n TEMP: %d \n", temp);
						pinOuts[i][pinOutsWidth] |= 0b00111111;
						pinOutsWidth++;
						pinOuts[i] = realloc(pinOuts[i], sizeof(char)*(pinOutsWidth+1));
						pinOuts[i][pinOutsWidth] = 0b01000000 | setPins(temp);
					}
					pinOutsWidth++;
				break;

				case 2:
					if(lvldB > 63)
						printf("\nWarning! Argument supplied on line %d to attenuator(s) 2 is larger than 63dB!\n", i);

					pinOuts[i][pinOutsWidth] = 0b01000000 | setPins(lvldB);
					pinOutsWidth++;
				break;

				case 3:
					if(lvldB > 63)
						printf("\nWarning! Argument supplied on line %d to attenuator(s) 3 is larger than 63dB!\n", i);

					pinOuts[i][pinOutsWidth] = 0b10000000 | setPins(lvldB);
					pinOutsWidth++;
				break;

				case 34:
					if(lvldB > 126)
						printf("\nWarning! Argument supplied on line %d to attenuator(s) 3 4 is larger than 126dB!\n", i);
					
					if(lvldB <= 63){
						int temp = lvldB / 2;
						pinOuts[i][pinOutsWidth] = 0b10000000 | setPins(temp);
						temp = lvldB - temp;
						pinOutsWidth++;
						pinOuts[i] = realloc(pinOuts[i], sizeof(char)*(pinOutsWidth+1));
						pinOuts[i][pinOutsWidth] = 0b11000000 | setPins(temp);
					}else{
						int temp = lvldB - 63;
						//printf("\n TEMP: %d \n", temp);
						pinOuts[i][pinOutsWidth] |= 0b01111111;
						pinOutsWidth++;
						pinOuts[i] = realloc(pinOuts[i], sizeof(char)*(pinOutsWidth+1));
						pinOuts[i][pinOutsWidth] = 0b11000000 | setPins(temp);
					}
					pinOutsWidth++;

				break;

				case 4:
					if(lvldB > 63)
						printf("\nWarning! Argument supplied on line %d to attenuator(s) 4 is larger than 63dB!\n", i);

					pinOuts[i][pinOutsWidth] = 0b11000000 | setPins(lvldB);
					pinOutsWidth++;
				break;

				case 123:
					if(lvldB > 63)
						printf("\nWarning! Argument supplied on line %d to attenuator(s) 123 is larger than 189dB!\n", i);

					pinOuts[i][pinOutsWidth] |= setPins(lvldB/3);
					pinOutsWidth++;
					pinOuts[i] = realloc(pinOuts[i], sizeof(char)*(pinOutsWidth+1));
					pinOuts[i][pinOutsWidth] = 0b01000000 | setPins(lvldB/3);
					pinOutsWidth++;
					pinOuts[i] = realloc(pinOuts[i], sizeof(char)*(pinOutsWidth+1));
					pinOuts[i][pinOutsWidth] = 0b10000000 | setPins(lvldB/3 + (lvldB % 3));
					//printf("\nHere we go!: %d", 65/3);
					pinOutsWidth++;
				break;

				case 1234:
					if(lvldB > 252)
						printf("\nWarning! Argument supplied on line %d to attenuator(s) 1234 is larger than 252dB!\n", i);
					
					pinOuts[i][pinOutsWidth] = 0b00000000 | setPins(lvldB/4);
					pinOutsWidth++;
					pinOuts[i] = realloc(pinOuts[i], sizeof(char)*(pinOutsWidth+1));
					pinOuts[i][pinOutsWidth] = 0b01000000 | setPins(lvldB/4);
					pinOutsWidth++;
					pinOuts[i] = realloc(pinOuts[i], sizeof(char)*(pinOutsWidth+1));
					pinOuts[i][pinOutsWidth] = 0b01000000 | setPins(lvldB/4);
					pinOutsWidth++;
					pinOuts[i] = realloc(pinOuts[i], sizeof(char)*(pinOutsWidth+1));
					pinOuts[i][pinOutsWidth] = 0b10000000 | setPins(lvldB/4 + (lvldB % 4));
					//printf("\nHere we go!: %d", 65/3);
					pinOutsWidth++;
				break;
			}
			}
			j++;
		}
		// if(i == 2)
		// 	printf("\nPIN: %d \n",pinOutsWidth);
		pinOuts[i] = realloc(pinOuts[i], sizeof(char)*(pinOutsWidth+1));
		pinOuts[i][pinOutsWidth] = '\0';
		
		pinOutsWidth = 0;
		j = 0;
	}
	
	j=0;
	for(int i = 0; i < INSTR_LENGTH; i++){
		printf("%d ",i);
		while(pinOuts[i][j] != '\0'){
			printf("  char: "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(pinOuts[i][j]));
			j++;
		}
		j=0;
		printf("\n");
	}


	//  this will send a signal to the arduino to tell it to behave in "file mode"
	char s[1] = { 'a' };
	char temp      [256];
	char buffer2   [256];
	char sig[1]  = { 0b11111100 };
	char sig2[1] = { 0b11111111 };
		
	for(int i = 0; i <INSTR_LENGTH; i++){
	//this is where the insturctions will be sent to the arduino
		if (!WriteFile(hDevice, s, 1, &bytesWritten, NULL)) {
			printf("\n\nALERT! Failed to send activation signal to arduino! Connection may have broken!\n\n", GetLastError());
		} else {
			printf("\n");
			printf("\nRead from file mode signal has been sent to attenuator.\n");
		}

		bool stop = 1;
		while (stop) {
			printf("\nWaiting for return... \n");
			printf("Arduino may need to be reset if this takes too long. \n");
			//if (ReadFile(hDevice, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
				//if (bytesRead == 1) {
					//if (buffer[0] == 'z') {
						printf("\nhello!\n");
						int numBytes = 0;

						while (pinOuts[i][numBytes] != '\0')
							numBytes++;

						WriteFile(hDevice, pinOuts[i], numBytes+1, &bytesWritten, NULL);
						delay(intInstr[i][0]);

						// while(1){
						// 	printf("SECOND WAIT,");
						// 	if (ReadFile(hDevice, buffer2, sizeof(buffer2) - 1, &bytesRead, NULL))
						// 	for(int i = 0; i < bytesRead; i++)
						// 		printf("%d", buffer2[i]);
						// }
						//printf("\n%d\nnum: ",numBytes);
						//WriteFile(hDevice, temp, numBytes + 1, &bytesWritten, NULL);

						// if (ReadFile(hDevice, buffer, sizeof(buffer) - 1, &bytesRead, NULL))
						// 	if(bytesRead > 0){
						// 		for (int i = 0; i < bytesRead; i++)
						// 			printf("%c", buffer[i]);
						// 	}
						// printf("\n");
					//}
					stop = 0;
				//}
			//}
		}
		//char tempChar = 'y';
		//WriteFile(hDevice, 'y', 1, &bytesWritten, NULL);
	}
}

char setPins(int lvldB){

	printf("\nlvl db: %d\n", lvldB);

	char s_Pins = 0b00000000;
	//printf("Hello from return pins: %d", lvldB);
	if (lvldB == 63) {
		return 0b01111111;
	} else {
	for (int k = 14; k >= 0; k--) {
		if(lvldB == 0){
			printf("Return pins: "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(s_Pins));
			return s_Pins;
		}
        if (pins[k] >= 1.0) {
            if (lvldB == (int)(pins[k] + 0.0000001)) {
                if (k == 6) {
                    s_Pins = s_Pins | 0b00100000;
                    lvldB = 0;
                    break;
                }
                if (k == 7) {
                    s_Pins = s_Pins | 0b00010000;
                    lvldB = 0;
                    break;
                }
                if (k == 8) {
                    s_Pins = s_Pins | 0b00001000;
                    lvldB = 0;
                    break;
                }
                if (k == 9) {
                    s_Pins = s_Pins | 0b00000100;
                    lvldB = 0;
                    break;
                }
                if (k == 10) {
                    s_Pins = s_Pins | 0b00000010;
                    lvldB = 0;
                    break;
                }
                if (k == 11) {
                    s_Pins = s_Pins | 0b00000001;
                    lvldB = 0;
                    break;
                }
            }
            if (lvldB % (int)(pins[k] + 0.0000001) != 0 && lvldB % (int)(pins[k] + 0.0000001) != lvldB) {
                switch (k) {
                    case 6:
                        //printf("k: %d\n",k);
                        s_Pins = s_Pins | 0b00100000;
                        lvldB -= (int)(pins[k] + 0.0000001);
                        break;
                    case 7:
                        //printf("k: %d\n",k);
                        s_Pins = s_Pins | 0b00010000;
                        lvldB -= (int)(pins[k] + 0.0000001);
                        break;
                    case 8:
                        //printf("k: %d\n",k);
                        s_Pins = s_Pins | 0b00001000;
                        lvldB -= (int)(pins[k] + 0.0000001);
                        break;
                    case 9:
                        //printf("k: %d\n",k);
                        s_Pins = s_Pins | 0b00000100;
                        lvldB -= (int)(pins[k] + 0.0000001);
                        break;
                    case 10:
                        //printf("k: %d\n",k);
                        s_Pins = s_Pins | 0b00000010;
                        lvldB -= (int)(pins[k] + 0.0000001);
                        break;
                    case 11:
                        //printf("k: %d\n",k);
                        s_Pins = s_Pins | 0b00000001;
                        lvldB -= (int)(pins[k] + 0.0000001);
                        break;
                    default:
                        break;
                }
            }
        }
    }
    //printf("char: "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(s_Pins));
	}

	return s_Pins;
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
			 
			if( lvldB2 == (int)(temp) ){

				if(i <= 7){
					writeData[0] = lookUp[i] | writeData[0];
					break;
				}else{
					writeData[1] = lookUp[i] | writeData[1];
					break;
				}
			}

			if(lvldB2 < 7 && lvldB2 > 3){
				writeData[0] = writeData[0] | lookUp[0];
				lvldB2 -= 6;
			}

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

		printf("\n\nHere is another test!: %d\n", lvldB2);

		if(lvldB2 <= 5)
			writeData[1] = writeData[1] | lookUp[14];

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
			}
		}
}

int convertToInteger(char* numStr) {
    int result = 0;
    int i = 0;

    while (numStr[i] != '\0') {
        result = result * 10 + (numStr[i] - '0');
        i++;
    }

    return result;
}
