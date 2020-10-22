#ifndef VARS_H_
#define VARS_H_
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct PCB
{
	struct PCB *Next_PCB;
	int PID;
	short int PC, P0, P1, P2, P3; // pointer registers 
	int IC; //number of instructions before preemption (i.e., time slice)
	int R0, R1, R2, R3; // general registers 
	char PSW[2]; // Process status word
	int ACC; // accumulator
	int BaseReg, LimitReg; // minimum and maximum memory location
	int Total_Pages;
	/* Note: You cannot store the process' memory in PCB */
};

struct PCB *RQ, *tmp, *RQT, *Current;

/*These are variables representing the VM itself*/

char IR[6];
short int PC;

short int P0; //these are the pointer registers
short int P1;
short int P2;
short int P3;

short int PRegs[4];
short int RRegs[4];

int R0; //GP regs
int R1;
int R2;
int R3;

int BaseRegister, LimitRegister;

int ACC;
char PSW[2];
char memory [100][6];     //this is the program memory for all programs
short int opcode;            

struct Program
{
	int Last;
	int Num_Lines;
	int  Data_Pages;
	int  Total_Pages;
	char temp_mem[100][6];
	struct PCB *New_PCB;
};

int Page_Table[10][5];
#endif