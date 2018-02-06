#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <string.h>

void main() {
	HANDLE readFromPipe, writeToPipe;
	char message[10];
	
	int bytesToWritten;
	readFromPipe = GetStdHandle(STD_INPUT_HANDLE);
	writeToPipe = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD read;
	//read whenever message is put on pipe from parent
	while (1) {
		//first read then write the pipe
		//inner whiles provide this 
		while (1) {
			if (!ReadFile(readFromPipe, message, sizeof(message), &read, NULL)) {
				printf("Unable to read message which is send from parent \n");
				system("pause");
				exit(0);
			}
			else {
				//exit inner while when message is received
				break;
			}
		}

		Sleep(atoi(message));
		char messageFromChild[100];
		
		while (1) {
			sprintf(messageFromChild, "ENDED", atoi(message));
			if (!WriteFile(writeToPipe, messageFromChild, sizeof(messageFromChild), &bytesToWritten, NULL)) {
				printf("unable to write to pipe from child \n");
				system("pause");
				exit(0);
			}
			else {
				//exit the inner while when message is send
				break;
			}
		}
	}
	CloseHandle(readFromPipe);
	CloseHandle(writeToPipe);
}