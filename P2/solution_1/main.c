#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Vars.h"

struct Semaphore Forks[5];
struct Semaphore Doorman;

//New functions for Project 2

int OS_Trap(char *, struct PCB *);
int Wait(struct PCB* , struct Semaphore *);
int Signal(struct Semaphore *);
int GetPID(struct PCB *);

//Previous Functions

void Create_PCBs() ;
void LoadPrograms() ;
struct PCB *GetNextProcess(struct PCB **Head);
void DeletePCB(struct PCB *Current);
void MvToTail (struct PCB *Current, struct PCB **RQT);
void PrintQ(struct PCB *Head);
void RestoreState(struct PCB *NextProc);
void SaveState(struct PCB **PrevProc);
//int ExecuteProc(struct PCB *CurrentProc);
void LoadProgram(int PID, struct PCB **tmp);


//Must declare Opcode Functions. Otherwise, end up with a boat load of
//highly annoying compiler warnings!


/*These variables are associated with the implementation of the VM*/
int fp;
int i, j, k;
int ProgSize;
char input_line [7];

int program_line = 0; //for loading program into memory

extern int ExecuteProc(struct PCB *);

/*
I made these global to make implementation a bit easier.
You do not have to.
*/

struct PCB *RQ, *tmp, *RQT, *Current;


int main()
{
	/*basically same as before. You are creating 5 philosopher processes (with identical code),
	and creating two separate PB programs: one with the array of fork semaphores and one where the doorman
	is added.

	Make sure you initialize the semaphores!!
	*/
	
	for(int i = 0; i < 5; i++) {
		Forks[i].count = 1;
		Forks[i].SemQ = NULL;
	}
	Create_PCBs();
	LoadPrograms();
	while(1)
	{
		Current = GetNextProcess(&RQ);
		RestoreState(Current);
		Current->IC = 1; //TQ1 requirement
		//Current->IC = (rand() % 10) + 1; //TQ2 requirement
		printf("CURRENT PID %d, IC %d\n", Current->PID, Current->IC);
		int Completed = ExecuteProc(Current);
		

		if (Completed == -1)
		{
			printf("Current Process is Blocked on Semaphore.\n");
			SaveState(&Current);
		}
 		
		if(Completed == 0)
		{
			SaveState(&Current);
			printf("Moving PID %d to TAIL\n", Current->PID);
			MvToTail(Current, &RQT);
			printf("RQT is %d\n", RQT->PID);
			if(RQ == NULL)
				RQ = RQT;
		}

		if (Completed == 1)
		{
			printf("Removing PID %d\n", Current->PID);
			DeletePCB(Current);
		}
		
		PrintQ(RQ) ;

		if (RQ == NULL)
			break;
	}
	return(0);
}

void Create_PCBs()
{    
	RQ = (struct PCB *) malloc (sizeof (struct PCB)) ;
        RQ->PID = 0;
        RQ->IC = 0;
	RQ->PC = 0;
	tmp = RQ ;
        for(i = 1; i < 5; i++)
               {tmp->Next_PCB = (struct PCB *) malloc (sizeof (struct PCB)) ;
                tmp->Next_PCB->PID = i ;
		tmp->Next_PCB->PC = 0;
                tmp->Next_PCB->IC = 0;
                tmp->Next_PCB->Next_PCB = NULL ;
                tmp = tmp->Next_PCB ;
               }

        RQT = tmp ;
        RQT->Next_PCB = NULL ;
}

void LoadPrograms()
{
	struct PCB *tmp ;
  	tmp = RQ ;
  	for (i = 0; i < 5 ; i++)
     	{
      		LoadProgram(i, &tmp) ;
      		printf("LimitReg = %d. IC = %d\n",tmp->LimitReg, tmp->IC) ;
      		tmp = tmp->Next_PCB ;
     	}
}

void LoadProgram(int PID, struct PCB **tmp)
{
	int i, fp ;
	int program_line = 100 * PID ;
	(*tmp)->BaseReg  = program_line ;
	(*tmp)->LimitReg = program_line + 99;
	ProgSize = program_line + 99;
	fp = open("arrayForks.txt", O_RDONLY) ; //always check the return value.
	printf("Open is %d\n", fp) ;

	if (fp < 0) //error in read
	{
		printf("Could not open file\n");
		exit(0) ;
	}

	int ret = read (fp, input_line, 7 ) ; //returns number of characters read`

	while (1)
	{
		if (ret <= 0) //indicates end of file or error
			break ; //breaks out of infinite loop

		printf("Copying Program line %d into memory\n", program_line) ;
		for (i = 0; i < 6 ; i++)
		{
			memory[program_line][i] = input_line[i] ;
			printf("%c ", memory[program_line][i]) ;
		}
		printf("\n") ;

		ret = read (fp, input_line, 7 ) ;
		program_line++ ; //now at a new line in the prog
	}

	printf("Read in Code. Closing File\n") ;
	close(fp) ;
}

/*
This function returns the PCB at the head of the RQ and updates
RQ to point to the next PCB in the list
*/

struct PCB *GetNextProcess(struct PCB **RQ)
{
	struct PCB * temp = *RQ;
	*RQ = (*RQ)->Next_PCB;
	temp->Next_PCB = NULL;
	return(temp) ;
}
       

/* Deletes the PCB (using free) */

void DeletePCB(struct PCB *Current)
{
	free(Current);
}       

/*
This function places the PCB pointed to by Current at the tail of the
Ready Queue and updates the RQT pointer.
*/

void MvToTail (struct PCB *Current, struct PCB **RQT)
{
	if (*RQT != NULL) {
		(*RQT)->Next_PCB = Current;
		*RQT = Current;
	}
	else
		*RQT = Current;
}


/* Prints out the elements of a linked list */

void PrintQ(struct PCB *Head)
{
	struct PCB * tmp = Head;
	while(tmp != NULL) {
		printf("PID: %d, IC: %d\n", tmp->PID, tmp->IC);
		tmp = tmp->Next_PCB;
	}
}


/*
This function restores the state of the process that is set to begin its
execution
*/

void RestoreState(struct PCB *NextProc)
{
	PRegs[0] = NextProc->P0;
	PRegs[1] = NextProc->P1;
	PRegs[2] = NextProc->P2;
	PRegs[3] = NextProc->P3;

	RRegs[0] = NextProc->R0;
	RRegs[1] = NextProc->R1;
	RRegs[2] = NextProc->R2;
	RRegs[3] = NextProc->R3;
	
	PC = NextProc->PC;
	PSW[0] = NextProc->PSW[0];
	PSW[1] = NextProc->PSW[1];
	ACC = NextProc->ACC;
	BaseRegister = NextProc->BaseReg;
	LimitRegister = NextProc->LimitReg;
}

/*
This function saves the state of the VM into the PCB of the process that
just completed its "time slice"
*/

void SaveState(struct PCB **PrevProc)
{
	(*PrevProc)->P0 = PRegs[0];
	(*PrevProc)->P1 = PRegs[1];
	(*PrevProc)->P2 = PRegs[2];
	(*PrevProc)->P3 = PRegs[3];
	
	(*PrevProc)->R0 = RRegs[0];
	(*PrevProc)->R1 = RRegs[1];
	(*PrevProc)->R2 = RRegs[2];
	(*PrevProc)->R3 = RRegs[3];

	(*PrevProc)->PC = PC;
	(*PrevProc)->PSW[0] = PSW[0];
	(*PrevProc)->PSW[1] = PSW[1];
	(*PrevProc)->ACC = ACC;
	(*PrevProc)->BaseReg = BaseRegister;
	(*PrevProc)->LimitReg = LimitRegister;
}