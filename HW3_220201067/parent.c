#include <stdio.h>
#include <Windows.h>
#include <stdlib.h>

//initial values
#define NUMBER_OF_PROCESSES 5
#define EXECUTION_TIME 5
#define ALPHA 0.5

//structure for processes
//all values is stored in this structure about time
//because they will be sorted according to predicted times which is tao_n
struct process {
	int processNo;
	int predicatedTime;
	int actualTime;
	int nextPredicatedTime;
};

//function prototypes
//all explanations about functions is the top of the function bodies
void initializeExecutionTimes(struct process pro[NUMBER_OF_PROCESSES]);
void sortByPredictedTimes(struct process pro[NUMBER_OF_PROCESSES]);
void printHelper(struct process pro[NUMBER_OF_PROCESSES],int printMethod);
void calculateActualLenghts(struct process pro[NUMBER_OF_PROCESSES]);
void calculateNextPredicatedTimes(struct process pro[NUMBER_OF_PROCESSES]);
void setNextValues(struct process pro[NUMBER_OF_PROCESSES]);
int calculatePredicatedTime(int preTime,int actTime);

int main() {

	int j = 0;
	//generate different random number for each time 
	//we have to put this line and this line use only one times
	srand(time(NULL));

	//initialize process structure array
	struct process processesExe[NUMBER_OF_PROCESSES];
	//fill the array according to given values
	initializeExecutionTimes(processesExe);
	
	
	STARTUPINFO si[NUMBER_OF_PROCESSES];
	PROCESS_INFORMATION pi[NUMBER_OF_PROCESSES];
	SECURITY_ATTRIBUTES sa[NUMBER_OF_PROCESSES];

	HANDLE processHandles[NUMBER_OF_PROCESSES];
	HANDLE writePipeFromParent[NUMBER_OF_PROCESSES], readPipeFromParent[NUMBER_OF_PROCESSES];
	HANDLE writePipeFromChild[NUMBER_OF_PROCESSES], readPipeFromChild[NUMBER_OF_PROCESSES];

	char* lpCommandLine[NUMBER_OF_PROCESSES] = { "child.exe", "child.exe", "child.exe", "child.exe", "child.exe" };


	int i = 0;

	//create processes and pipes
	for (i = 0; i < NUMBER_OF_PROCESSES; i++) {
		SecureZeroMemory(&sa[i], sizeof(SECURITY_ATTRIBUTES));
		sa[i].bInheritHandle = TRUE;
		sa[i].lpSecurityDescriptor = NULL;
		sa[i].nLength = sizeof(SECURITY_ATTRIBUTES);

		//Creating pipe for parent
		if (!CreatePipe(&readPipeFromParent[i], &writePipeFromParent[i], &sa[i], 0)) {
			printf("An error had been occured while creating pipe for parent with error code %d /n ", GetLastError());
			system("pause");
			exit(0);
		}

		//Create pipe for child
		if (!CreatePipe(&readPipeFromChild[i], &writePipeFromChild[i], &sa[i], 0)) {
			printf("An error had been occured while creating pipe for child with error code %d /n ", GetLastError());
			system("pause");
			exit(0);
		}

		//creating variables for child process
		SecureZeroMemory(&si[i], sizeof(STARTUPINFO));
		SecureZeroMemory(&pi[i], sizeof(PROCESS_INFORMATION));

		//assign necessary handles to necessary in out part of pipes
		si[i].cb = sizeof(STARTUPINFO);
		si[i].hStdInput = readPipeFromParent[i];
		si[i].hStdOutput = writePipeFromChild[i];
		si[i].hStdError = GetStdHandle(STD_ERROR_HANDLE);
		si[i].dwFlags = STARTF_USESTDHANDLES;

		if (!CreateProcess(NULL,
			lpCommandLine[i],
			NULL,
			NULL,
			TRUE,
			CREATE_NO_WINDOW,
			NULL,
			NULL,
			&si[i],
			&pi[i]))
		{
			printf("Child process can not be created!!! \n");
			system("pause");
			exit(0);
		}
		else {
			processHandles[i] = pi[i].hProcess;
		}
	}

	char messageFromParent[10];
	char messageFromChild[100];
	int bytesToWrite = 0;
	int bytesWritten = 0;
	int indexOfPipe;
	//we have to send message and receive message 
	//on the pipe up to  the execution time value
	for (j = 0; j < EXECUTION_TIME; j++) {

		//I used for loop for doing executions and 
		//beacuse of the indexs of the array
		//I have to sort list according to predicted values
		//but I also dont want to lose process index 
		//beacuse of this I create the structure array for the processes
		//this array is sorted according to predicted values 
		//and by this way I dont lose process index this process index used for pipe index to send message or recieve 
		sortByPredictedTimes(processesExe);
		//calculate actual lenghts before send via pipe beacuse
		//child process sleep this time period
		calculateActualLenghts(processesExe);

		//print execution order
		printf("%d. execution order <%d,%d,%d,%d,%d>\n",
			j + 1, processesExe[0].processNo,
			processesExe[1].processNo, processesExe[2].processNo,
			processesExe[3].processNo, processesExe[4].processNo);
		
		for (i = 0; i < NUMBER_OF_PROCESSES; i++)
		{
			//check the pipe 
			//I can use for loop because processExe array has been sorted 
			//and I can reach in order 0 1 2 3.... but process numbers different for each
			sprintf(messageFromParent, "%d", processesExe[i].actualTime);
			
			bytesToWrite = strlen(messageFromParent);
			//use i. process's process number-1 in the processesExe array
			//-1 because process numbers start with 1 ends with 5
			//but pipe indexs start with 0 end with 4
			indexOfPipe = processesExe[i].processNo - 1;
			while (1) {
				// WriteFile function writes data to the specified file or input / output(I / O) device.
				if (!WriteFile(writePipeFromParent[indexOfPipe], messageFromParent, bytesToWrite, &bytesWritten, NULL))
				{
					printf("unable to write to pipe parent\n");
					system("pause");
					exit(0);
				}
				else {
					//when the write pipe exit the while loop
					break;
				}
			}
			printf("P%d STARTED\n", processesExe[i].processNo);
			while (1) {
				//indexOfPipe = processesExe[i].processNo - 1;
				if (!ReadFile(readPipeFromChild[indexOfPipe], messageFromChild, sizeof(messageFromChild), &bytesWritten, NULL)) {
					printf("unable to read from pipe which is child!!!! \n");
					system("pause");
					exit(0);
				}
				else {
					//when the read from pipe exit the while loop
					break;
				}
			}
			printf("P%d %s\n", processesExe[i].processNo, messageFromChild);
		}
		
		calculateNextPredicatedTimes(processesExe);
		printHelper(processesExe, 0);
		printHelper(processesExe, 1);
		printHelper(processesExe, 2);
		//set next predicated time to new predicated time 
		//for next execution
		setNextValues(processesExe);
	}
	
	
	//close handles
	for (i = 0; i < NUMBER_OF_PROCESSES; i++) {

		CloseHandle(readPipeFromParent[i]);
		CloseHandle(writePipeFromParent[i]);
		CloseHandle(readPipeFromChild[i]);
		CloseHandle(writePipeFromChild[i]);
		CloseHandle(pi[i].hThread);
		CloseHandle(pi[i].hProcess);
	}

	system("pause");
	exit(0);
	return 0;
}

//fill the structure according to given values for initialize
void initializeExecutionTimes(struct process pro[NUMBER_OF_PROCESSES]) {
	pro[0].processNo = 1;
	pro[0].predicatedTime = 300;
	pro[0].actualTime = 0;

	pro[1].processNo = 2;
	pro[1].predicatedTime = 220;
	pro[1].actualTime = 0;

	pro[2].processNo = 3;
	pro[2].predicatedTime = 180;
	pro[2].actualTime = 0;

	pro[3].processNo = 4;
	pro[3].predicatedTime = 45;
	pro[3].actualTime = 0;

	pro[4].processNo = 5;
	pro[4].predicatedTime = 255;
	pro[4].actualTime = 0;
}

//sort struct array according to predicted times
void sortByPredictedTimes(struct process pro[NUMBER_OF_PROCESSES]) {
	int i,j;
	for (i = 0; i < NUMBER_OF_PROCESSES; i++) {
		for (j = i; j < NUMBER_OF_PROCESSES; j++) {
			if (pro[j].predicatedTime < pro[i].predicatedTime) {
				struct process temp = pro[i];
				pro[i] = pro[j];
				pro[j] = temp;
			}
		}
	}
}

//calculated next predicted time according to formula 
//this function takes two arguments one of them is predicated time
//another is actual executing time which is generated 
//by the calculateActualLenghts function for each processes
int calculatePredicatedTime(int preTime,int actTime) {
	return ALPHA*actTime + (1 - ALPHA)*preTime;
}

//print the each step 
//this function takes two argument one of them is the struct array
//another one is the print method
//there are 3 print method which is indicate by 0,1 and 2
//I think this vay because I dont want to execute different function for each
//instead of the 3 function I collected them in one function by this way
void printHelper(struct process pro[NUMBER_OF_PROCESSES], int printMethod) {
	int i;
	printf("PROCESS \t TAO \t T(Actual Time) \t TAO(Next Value)\n");
	printf("--------------------------------------------------------\n");
	switch (printMethod)
	{
	case 0:
		for (i = 0; i < NUMBER_OF_PROCESSES; i++)
			printf("%d \t\t %d \t\t - \t\t - \n",pro[i].processNo,pro[i].predicatedTime);
		break;
	case 1:
		for (i = 0; i < NUMBER_OF_PROCESSES; i++)
			printf("%d \t\t %d \t\t %d \t\t - \n", pro[i].processNo, pro[i].predicatedTime,pro[i].actualTime);
		break;
	case 2:
		for (i = 0; i < NUMBER_OF_PROCESSES; i++)
			printf("%d \t\t %d \t\t %d \t\t %d \n", pro[i].processNo, pro[i].predicatedTime, pro[i].actualTime,pro[i].nextPredicatedTime);
		break;
	default:
		break;
	}
}

//generate random execution time for each process 
void calculateActualLenghts(struct process pro[NUMBER_OF_PROCESSES]) {
	int i,randomNumber;
	for (i = 0; i < NUMBER_OF_PROCESSES; i++) {
		
		randomNumber = 50 + (rand()%250);
		pro[i].actualTime = randomNumber;
	}
}

//assign next predicted value for each process 
void calculateNextPredicatedTimes(struct process pro[NUMBER_OF_PROCESSES]) {
	int i;
	for (i = 0; i < NUMBER_OF_PROCESSES; i++) {
		pro[i].nextPredicatedTime = calculatePredicatedTime(pro[i].predicatedTime, pro[i].actualTime);
	}
}

//set next predicted value as predicted value for each process
void setNextValues(struct process pro[NUMBER_OF_PROCESSES]) {
	int i;
	for (i = 0; i < NUMBER_OF_PROCESSES; i++)
		pro[i].predicatedTime = pro[i].nextPredicatedTime;
}