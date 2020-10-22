#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Vars.h"

extern void Print_Page_Table(int);
struct PCB *New_Program;
extern void Free_Pages(struct PCB *);
extern void printMEM(int);
extern struct PCB *Admit_Program();
void RestoreState(struct PCB *);
int  ExecuteProc(struct PCB *);
void DeletePCB(struct PCB *);
void MvToTail(struct PCB *, struct PCB **);
void SaveState(struct PCB **);
void PrintQ(struct PCB*);
struct PCB* GetNextProcess(struct PCB **);
void Place_On_Queue(struct PCB *);

extern int ExecuteProc(struct PCB *); 

int Max_Line = 0;

/*These are variables representing the VM itself*/

int program_line = 0; // For loading program into Memory

/*These variables are associated with the implementation of the VM*/
int fp; 
int i; 
int j, k; 
char input_line [7];

int main(int argc, char *argv[])
{
	RQ = NULL; RQT = NULL; /* Changed how PCB allocated and placed on RQ */
	while(1)
	{
		New_Program = Admit_Program(); 
		if(New_Program == NULL)
			break;
		printf("Putting PID %d on the RQ\n", New_Program->PID);
		Place_On_Queue(New_Program);
	}

	while(1)
	{
		Current = GetNextProcess(&RQ); //Standard and already coded
		RestoreState(Current); //Restores the status of the registers, all registers, PSW, ACC, PC, etc
		Current->IC = (rand() % 200) + 5; //Assigns a random time slice for the program that was loaded
		printf("CURRENT PID %d, IC %d\n\n", Current->PID, Current->IC);
		int Completed = ExecuteProc(Current); // checks to see if the execute function returns a 1 (true)  
		if (Completed)
		{
			printf("The program in PCB %d has completed its execution and will be terminated\n",Current->PID);
			printf("Removing PID %d\n\n", Current->PID); //Program has completed execution and will terminate
			printf("Pages freed from PID %d:\n", Current->PID);
			Print_Page_Table(Current->PID);
			Free_Pages(Current);
			DeletePCB(Current); //Calls DeletePCB, expanded upon below

			while(1)
			{
				New_Program = Admit_Program();
				if(New_Program == NULL)
					break;
				printf("Putting PID %d on the RQ\n", New_Program->PID);
				printf("PID is %d PC is %d\n", New_Program->PID, New_Program->PC);
				Place_On_Queue(New_Program);
			}
		}
		else
		{
			SaveState(&Current); 
			printf("Moving PID %d to TAIL\n", Current->PID); //Moves the PCB back to the end of the queue
			MvToTail(Current, &RQT); 
			printf("RQT is %d\n", RQT->PID); 
		}
		PrintQ(RQ); //Prints the state of the ready queue
		sleep(1); 
		if (RQ == NULL) //If RQ is NULL this breaks the while loop and we're done
			break;

	}
	printMEM(100); // prints out the final state of the main memory
	printf("\nPage Assignments:\n"); //print page assignments from program
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 5; j++) {
			int f = Page_Table[i][j];
			if (f > -1)
				printf("PID [%d] Frame [%d]: MemPage[%d]\n", i, j, f);
		}
		printf("\n");
	}
}

/*	This function returns the PCB at the head of the RQ and updates
	RQ to point to the next PCB in the list
*/

struct PCB *GetNextProcess(struct PCB **RQ)
{
	struct PCB * temp = *RQ;
	*RQ = (*RQ)->Next_PCB;
	temp->Next_PCB = NULL;
	return(temp) ;
}
       

/*	Deletes the PCB (using free) */

void DeletePCB(struct PCB *Current)
{
	free(Current);
}       

/*	This function places the PCB pointed to by Current at the tail of the
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


/*	Prints out the elements of a linked list */

void PrintQ(struct PCB *Head)
{
	struct PCB * tmp = Head;
	while(tmp != NULL) {
		printf("PID: %d, IC: %d\n", tmp->PID, tmp->IC);
		tmp = tmp->Next_PCB;
	}
}


/*	This function restores the state of the process that is set to begin its
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
}

/*	This function saves the state of the VM into the PCB of the process that
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
}

void Place_On_Queue(struct PCB *newPro)
{
	MvToTail(newPro, &RQT);
	if (RQ == NULL)
		RQ = RQT;
}