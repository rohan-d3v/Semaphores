/*
* Rohan Krishna Ramkhumar
* rxr353
* as6- Baboon Crossing Problem Using Semaphores
* Reference Used: http://eecs-00to.case.edu/338.S17/old%to0assignments%to0and%to0exams.html
* Concurrent Programming with linux processes, System V Semaphores: Spring 2015 Solutions
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//Serves as Semaphore and shm key
//Define did not work so I just declared it universally. Sorry
key_t semkey = 0xFA2B;

//the mutex,  A to B semaphores, and vice versa
//Same declaration as psudocode
#define mutex	0
#define semtob	1
#define semtoa	2
#define semnum	3
//The Baboons going to B and baboons going to A

#define baboontob	1
#define baboontoa	2

//Stalltime between crossing for empty loop iterations
#define stalltime	5000
#define crosstime	10

#define debug 1//Detailed Crossing info

void babfork(int aOrBBnd);//New baboon process

//Baboon going toward a and B respectively
void a();
void b();

//Semaphores used
void semWait(int semid, int semno);
void semSignal(int semid, int semno);
int createSem(int value);

//Stall function for waiting for blank loop iterations
void stall(int iterations);

int get_semid(key_t semkey);
int get_shmid(key_t shmkey);

//semaphore control union
union semun{
	int val;				//setval value
	struct semid_ds *buf;	//IPC_STAT and IPC_SET Buffer
	unsigned short *array;	//GETALL and SETALL
	struct seminfo *bufto;	//IPC_INFO buffer
};

struct varshare {//Memory struct to store all crossing, a and b variable
	int XingCnt;
	int XedCnt;
	int toaWaitCnt;
	int tobWaitCnt;

	enum {None, aBnd, bBnd} XingDir;
};

void debugprint(struct varshare * shared);//It doesn't work for some reason without this

int main(int argc, char *argv[]){

	printf("Current PID: %d \n", getpid());//Starting PID
	fflush(stdout);

	if (argc != 2){//Executable file input
		printf("Please run program with number and order of baboons\n");
		printf("a for going to a and b for going to b \n");
		fflush(stdout);
	}

	printf("Argument passed: %s \n", argv[1]);//Passed argument for reference
	fflush(stdout);

	union semun semval;

	unsigned short semInitVal[semnum];
	semInitVal[mutex] = 1;
	semInitVal[semtob] = 0;
	semInitVal[semtoa] = 0;
	semval.array = semInitVal;

	int semid = get_semid((key_t)semkey);
	if (semctl(semid, mutex, SETALL, semval) == -1){
		perror("semctl failed");
		exit(EXIT_FAILURE);
	}
	int shmid = get_shmid((key_t)semkey);
	struct varshare * varshared = shmat(shmid,0,0);

	varshared->XingCnt = 0;
	varshared->XedCnt = 0;
	varshared->toaWaitCnt = 0;
	varshared->tobWaitCnt = 0;
	varshared->XingDir = None;

	int i = 0;
	while (argv[1][i] != 0){
		switch (argv[1][i]) {
			case 'b':
			case 'B':
				babfork(baboontob);
				break;
	
			case 'a':
			case 'A':
				babfork(baboontoa);
				break;
			
			default:
				printf("Invalid argument! Try a, A or b, B.\n");
				fflush(stdout);
				exit(EXIT_FAILURE);
				break;
			
		}

		stall(stalltime);
		i++;
	}

	int j;
	for (j = 0; j < i; j++) {
		wait(NULL);
	}

	printf("That's all folks!\n");
	fflush(stdout);
if (shmdt(varshared) == -1) {
perror("shmdt failed");
exit(EXIT_FAILURE);
}

if (shmctl(shmid, IPC_RMID, NULL) < 0) {
perror("shmctrl failed");
exit(EXIT_FAILURE);
}

if (semctl(semid, mutex, IPC_RMID, semval) == -1) {
perror("semctl failed");
exit(EXIT_FAILURE);
}
}	

void babfork (int aOrBBnd){
	pid_t child;
	child = fork();
	if (child == -1){
		perror ("Fork Failure");
		exit(EXIT_FAILURE);
	}

	else if (child == 0){
		if (aOrBBnd == baboontoa)
			a();
		else if (aOrBBnd == baboontob)
			b();
		else{
			printf ("Invalid baboon \n");
			fflush(stdout);
			exit(EXIT_FAILURE);
		}
	}
	else
		return;
}

void debugprint(struct varshare *shared){
	if(debugprint > 0){
		int XingCnt;
		int XedCnt;
		int toaWaitCnt;
		int tobWaitCnt;
		enum {None, aBnd, bBnd} XingDir;
		
		XingCnt = shared->XingCnt;
		XedCnt = shared->XedCnt;
		toaWaitCnt = shared->toaWaitCnt;
		tobWaitCnt = shared->tobWaitCnt;
		XingDir = shared->XingDir;
		
		printf("PID: %d | XingCnt = %d, XedCnt = %d, toaWaitCnt = %d, tobWaitCnt = %d, Dir = %d\n", 
			getpid(), XingCnt, XedCnt, toaWaitCnt, tobWaitCnt, XingDir);
		fflush(stdout);
	}
}

void a(void){
	printf("PID: %d \n Baboon Crossing to A\n", getpid());
	fflush(stdout);

	int semid = get_semid((key_t)semkey);
	int shmid = get_shmid((key_t)semkey);
	struct varshare * varshared = shmat(shmid, 0, 0);

	printf("Waiting on Mutex for pass to A\n");
	fflush(stdout);
	semWait(semid, mutex);
	printf("Mutex Passed");
	fflush(stdout);

	if ((varshared->XingDir == aBnd || varshared->XingDir == None) && 
		varshared->XingCnt < 5 &&
		(varshared->XingCnt + varshared->XingCnt < 10 || varshared->tobWaitCnt == 0) &&
		varshared->toaWaitCnt == 0){
		printf ("Crossing to A about to start\n");
		fflush(stdout);
		varshared->XingDir = aBnd;
		varshared->XingCnt++;

		debugprint(varshared);
		printf("A signaling mutex \n");
		fflush(stdout);
		semSignal(semid, mutex);
	}

	else{
		printf("A is now supposed to wait\n");
		fflush(stdout);
		varshared->toaWaitCnt++;
		debugprint(varshared);

		printf("Signalling mutex \n");
		fflush(stdout);
		semSignal(semid, mutex);
		semWait(semid, semtoa);

		printf("A was waiting until signal call. Signal has been called \n");
		fflush(stdout);
		varshared->toaWaitCnt--;
		varshared->XingCnt++;
		varshared->XingDir = aBnd;

		debugprint(varshared);

		if (varshared->toaWaitCnt > 0 &&
			varshared->XingCnt < 5 &&
			(varshared->XedCnt+ varshared->XingCnt < 10 ||
				varshared->tobWaitCnt == 0)){
			printf("Signaling baboon behind current baboon \n");
			printf("Baboon about to cross\n");
			fflush(stdout);
			semSignal(semid, semtoa);
		}

		else{
			printf("Baboon about to cross \n");
			fflush(stdout);
			debugprint(varshared);
			printf("Signaling Mutex\n");
			fflush(stdout);
			semSignal(semid, mutex);
		}
	}

	printf (" Rope Crossing\n");
	fflush(stdout);
	stall(crosstime);

	printf("Rope Crossed. Waiting for mutex \n");
	fflush(stdout);
	semWait(semid, mutex);
	printf("Mutex Passed\n");
	fflush(stdout);
	varshared->XedCnt++;
	varshared->XingCnt--;

	if (varshared->toaWaitCnt!= 0 &&
		(varshared->XingCnt + varshared->XedCnt < 10 ||
			varshared->tobWaitCnt == 0)){
		debugprint(varshared);
		printf("Signaling Baboon Crossing to A\n");
		semSignal(semid, semtoa);
	}

	else if(varshared->XingCnt == 0 &&
		varshared->tobWaitCnt !=0 &&
		(varshared->toaWaitCnt == 0 ||
			varshared->XedCnt + varshared->tobWaitCnt >= 10)){
		printf("Changing Direction to turn toward B\n");
		printf("Signaling Baboon waiting to cross to B\n");
		fflush(stdout);

		varshared->XingDir = bBnd;
		varshared->XedCnt = 0;
		debugprint(varshared);
		semSignal(semid, semtob);
	}

	else if(varshared->XingCnt == 0 &&
		varshared->toaWaitCnt == 0 && 
		varshared -> tobWaitCnt == 0){
		printf("Crossing Direction set to none. No more Baboons waiting \n");
		fflush(stdout);
		varshared->XingDir = None;
		varshared->XedCnt = 0;

		debugprint(varshared);
		printf("Signaling Mutex\n");
		fflush(stdout);
		semSignal(semid, mutex);
	}

	else
	{
		debugprint(varshared);
		printf("Signaling Mutex\n");
		fflush(stdout);
		semSignal(semid, mutex);
	}

	if(shmdt(varshared)==-1){
		perror("shmdt failed");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}

void b(void){
	printf("PID: %d \n Baboon Crossing to B\n", getpid());
	fflush(stdout);

	int semid = get_semid((key_t)semkey);
	int shmid = get_shmid((key_t)semkey);
	struct varshare * varshared = shmat(shmid, 0, 0);

	printf("Waiting on Mutex for pass to B\n");
	fflush(stdout);
	semWait(semid, mutex);
	printf("Mutex Passed");
	fflush(stdout);
	debugprint(varshared);

	if ((varshared->XingDir == bBnd || varshared->XingDir == None) && 
		varshared->XingCnt < 5 &&
		(varshared->XingCnt + varshared->XingCnt < 10 || varshared->toaWaitCnt == 0) &&
		varshared->tobWaitCnt == 0){
		printf ("Crossing to B about to start\n");
		fflush(stdout);
		varshared->XingDir = bBnd;
		varshared->XingCnt++;

		debugprint(varshared);
		printf("B signaling mutex \n");
		fflush(stdout);
		semSignal(semid, mutex);
	}

	else{
		printf("B is now supposed to wait\n");
		fflush(stdout);
		varshared->tobWaitCnt++;
		debugprint(varshared);

		printf("Signaling mutex \n");
		fflush(stdout);
		semSignal(semid, mutex);
		semWait(semid, semtob);

		printf("B was waiting until signal call. Signal has been called \n");
		fflush(stdout);
		varshared->tobWaitCnt--;
		varshared->XingCnt++;
		varshared->XingDir = bBnd;

		debugprint(varshared);

		if (varshared->tobWaitCnt > 0 &&
			varshared->XingCnt < 5 &&
			(varshared->XedCnt+ varshared->XingCnt < 10 ||
				varshared->toaWaitCnt == 0)){
			printf("Signaling baboon behind current baboon \n");
			printf("Baboon about to cross\n");
			fflush(stdout);
			semSignal(semid, semtoa);
		}

		else{
			printf("Baboon about to cross \n");
			fflush(stdout);
			debugprint(varshared);
			printf("Signaling Mutex\n");
			fflush(stdout);
			semSignal(semid, mutex);
		}
	}

	printf (" Rope Crossing\n");
	fflush(stdout);
	stall(crosstime);

	printf("Rope Crossed. Waiting for mutex \n");
	fflush(stdout);
	semWait(semid, mutex);
	printf("Mutex Passed\n");
	fflush(stdout);
	varshared->XedCnt++;
	varshared->XingCnt--;

	if (varshared->tobWaitCnt!= 0 &&
		(varshared->XingCnt + varshared->XedCnt < 10 ||
			varshared->tobWaitCnt == 0)){
		debugprint(varshared);
		printf("Signaling Baboon Crossing to A\n");
		semSignal(semid, semtoa);
	}

	else if(varshared->XingCnt == 0 &&
		varshared->toaWaitCnt !=0 &&
		(varshared->tobWaitCnt == 0 ||
			varshared->XedCnt + varshared->tobWaitCnt >= 10)){
		printf("Changing Direction to turn toward a\n");
		varshared->XingDir = aBnd;
		varshared->XedCnt = 0;
		debugprint(varshared);
		printf("Signaling Baboon waiting to cross to a\n");
		fflush(stdout);
		semSignal(semid, semtoa);
	}

	else if(varshared->XingCnt == 0 &&
		varshared->toaWaitCnt != 0 && 
		varshared -> tobWaitCnt == 0){
		printf("Crossing Direction set to none. No more Baboons waiting \n");
		fflush(stdout);
		varshared->XingDir = None;
		varshared->XedCnt = 0;

		debugprint(varshared);
		printf("Signaling Mutex\n");
		fflush(stdout);
		semSignal(semid, mutex);
	}

	else
	{
		debugprint(varshared);
		printf("Signaling Mutex\n");
		fflush(stdout);
		semSignal(semid, mutex);
	}

	if(shmdt(varshared)==-1){
		perror("shmdt failed");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}

void stall(int iterations){
	int i;
	for (i = 0; i< iterations; i++){
		;
	}
}

void semWait(int semid, int semno){
	struct  sembuf waitbuf;

	waitbuf.sem_num = semno;

	waitbuf.sem_op = -1;
	waitbuf.sem_flg = 0;

	if (semop(semid, &waitbuf, 1) == -1){
		perror("semWait failed");
		exit(EXIT_FAILURE);
	}
}

void semSignal(int semid, int semno){
	struct sembuf sigBuf;

	sigBuf.sem_num = semno;

	sigBuf.sem_op = 1;
	sigBuf.sem_flg = 0;

	if (semop(semid, &sigBuf, 1) == -1) {
		perror("semSignal failed");
		exit(EXIT_FAILURE);
	}
}

int get_semid(key_t semkey){
	int value = semget(semkey, semnum, 0777 | IPC_CREAT);

	if (value == -1){
		perror("semget failed");
		exit(EXIT_FAILURE);
	}

	return value;
}

int get_shmid(key_t shmkey){
	int value = shmget(shmkey, sizeof(struct varshare), 0777 | IPC_CREAT);

	if (value == -1){
		perror("semget failed");
		exit(EXIT_FAILURE);
	}

	return value;
}
