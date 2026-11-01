/*
    Name 1: Anmol Dubey
    UTEID 1: ad56328
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
    IRD,
    COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1, PCMUX0,
    DRMUX,
    SR1MUX,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
/* MODIFY: you have to add all your new control signals */
    /* Lab 4 signals: bits 35-49, same order as the ucode4 columns */
    COND2,                /* bit 35: COND2, the MSB of the 3-bit COND field */
    EXC_CHECK,                /* bit 36: EXC_CHECK, enables the exception-detect logic */
    LD_PSR, LD_SSP, LD_USP, LD_VECTOR,  /* bits 37-40: LD_PSR, LD_SSP, LD_USP, LD_VECTOR, in that order */
    GATE_PSR, GATE_SP, GATE_VECTOR,        /* bits 41-43: GATE_PSR, GATE_SP, GATE_VECTOR, in that order */
    PSRMUX,                /* bit 44: PSRMUX, 0 = load PSR from BUS (RTI), 1 = set supervisor */
    SR1MUX1,                /* bit 45: SR1MUX1, the new high bit of SR1MUX (10 selects R6) */
    DRMUX1,                /* bit 46: DRMUX1, the new high bit of DRMUX (10 selects R6) */
    SPMUX1, SPMUX0,              /* bits 47-48: SPMUX1 then SPMUX0 (00 R6+2, 01 R6-2, 10 SSP, 11 USP) */
    VECTORMUX,                /* bit 49: VECTORMUX, 0 = INTV, 1 = EXCV */
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
				      (x[J3] << 3) + (x[J2] << 2) +
				      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); } 
int GetLSHF1(int *x)         { return(x[LSHF1]); }
/* MODIFY: you can add more Get functions for your new control signals */
/* Lab 4 getters */
int GetCOND3(int *x)         { return((x[COND2] << 2) + (x[COND1] << 1) + x[COND0]); }  /* (x[COND2] << 2) + (x[COND1] << 1) + x[COND0]: full 3-bit COND */
int GetSR1MUX2(int *x)       { return((x[SR1MUX1] << 1) + x[SR1MUX]); }  /* (x[SR1MUX1] << 1) + x[SR1MUX]: 0 = 11.9, 1 = 8.6, 2 = R6 */
int GetDRMUX2(int *x)        { return((x[DRMUX1] << 1) + x[DRMUX]); }  /* (x[DRMUX1] << 1) + x[DRMUX]: 0 = 11.9, 1 = R7, 2 = R6 */
int GetSPMUX(int *x)         { return((x[SPMUX1] << 1) + x[SPMUX0]); }  /* (x[SPMUX1] << 1) + x[SPMUX0]: 0 = R6+2, 1 = R6-2, 2 = SSP, 3 = USP */
int GetEXC_CHECK(int *x)     { return(x[EXC_CHECK]); }  /* the single bit x[EXC_CHECK] */
int GetLD_PSR(int *x)        { return(x[LD_PSR]); }  /* the single bit x[LD_PSR] */
int GetLD_SSP(int *x)        { return(x[LD_SSP]); }  /* the single bit x[LD_SSP] */
int GetLD_USP(int *x)        { return(x[LD_USP]); }  /* the single bit x[LD_USP] */
int GetLD_VECTOR(int *x)     { return(x[LD_VECTOR]); }  /* the single bit x[LD_VECTOR] */
int GetGATE_PSR(int *x)      { return(x[GATE_PSR]); }  /* the single bit x[GATE_PSR] */
int GetGATE_SP(int *x)       { return(x[GATE_SP]); }  /* the single bit x[GATE_SP] */
int GetGATE_VECTOR(int *x)   { return(x[GATE_VECTOR]); }  /* the single bit x[GATE_VECTOR] */
int GetPSRMUX(int *x)        { return(x[PSRMUX]); }  /* the single bit x[PSRMUX]: 0 = from BUS, 1 = supervisor */
int GetVECTORMUX(int *x)     { return(x[VECTORMUX]); }  /* the single bit x[VECTORMUX]: 0 = INTV, 1 = EXCV */

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x08000 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */

int READY;	/* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it 
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */ 

/* For lab 4 */
int INTV; /* Interrupt vector register */
int EXCV; /* Exception vector register */
int SSP; /* Initial value of system stack pointer */
/* MODIFY: You may add system latches that are required by your implementation */
int PSR15;   /* PSR15: privilege bit PSR[15], 1 = user, 0 = supervisor */
int USP;   /* USP: saved user stack pointer, loaded from R6 on a user-to-supervisor switch */
int VECTOR;   /* VECTOR: active 8-bit vector, loaded from INTV or EXCV at entry */
int INT;   /* INT: interrupt-pending flag, set by the timer at cycle 300, cleared when taken */

} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {                                                    
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {   
  /* Lab 4: simulated timer device, raises one interrupt at cycle 300 (clarifications 3, 6, 16) */
  if (CYCLE_COUNT == 300) /* CYCLE_COUNT equals 300 */
  {     
    CURRENT_LATCHES.INT = 1;         /* set CURRENT_LATCHES.INT to 1: the microsequencer sees it this cycle */
    NEXT_LATCHES.INT = 1;         /* set NEXT_LATCHES.INT to 1: survives the CURRENT = NEXT commit at the end of this cycle */
    CURRENT_LATCHES.INTV = 0x01;         /* set CURRENT_LATCHES.INTV to 0x01: the timer's interrupt vector */
    NEXT_LATCHES.INTV = 0x01;         /* set NEXT_LATCHES.INTV to 0x01 */
  }                                             

  eval_micro_sequencer();   
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
    int i;

    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
	if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
	}
	cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
	cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
    int address; /* this is a byte address */

    printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
    int k; 

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
    case 'G':
    case 'g':
	go();
	break;

    case 'M':
    case 'm':
	scanf("%i %i", &start, &stop);
	mdump(dumpsim_file, start, stop);
	break;

    case '?':
	help();
	break;
    case 'Q':
    case 'q':
	printf("Bye.\n");
	exit(0);

    case 'R':
    case 'r':
	if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
	else {
	    scanf("%d", &cycles);
	    run(cycles);
	}
	break;

    default:
	printf("Invalid Command\n");
	break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */ 
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	    printf("Error: Too few lines (%d) in micro-code file: %s\n",
		   i, ucode_filename);
	    exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;

	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	    /* Needs to find enough bits in line. */
	    if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
		       ucode_filename, i);
		exit(-1);
	    }
	    if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
		       ucode_filename, i, j);
		exit(-1);
	    }

	    /* Set the bit in the Control Store. */
	    CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
	    index++;
	}

	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	    printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		   ucode_filename, i);
    }
    printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
    int i;

    for (i=0; i < WORDS_IN_MEM; i++) {
	MEMORY[i][0] = 0;
	MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
    FILE * prog;
    int ii, word, program_base;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
	printf("Error: Can't open program file %s\n", program_filename);
	exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
	program_base = word >> 1;
    else {
	printf("Error: Program file is empty\n");
	exit(-1);
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
	/* Make sure it fits. */
	if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
		   program_filename, ii);
	    exit(-1);
	}

	/* Write the word to memory array. */
	MEMORY[program_base + ii][0] = word & 0x00FF;
	MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
	ii++;
    }

    if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *argv[], int num_prog_files) { 
    int i;
    init_control_store(argv[1]);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(argv[i + 2]);
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
    CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */
    CURRENT_LATCHES.PSR15 = 1;       /* set CURRENT_LATCHES.PSR15 to 1: start in user mode (clarification 7) */

    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 3) {
	printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
	       argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv, argc - 2);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
	printf("Error: Can't open dumpsim file\n");
	exit(-1);
    }

    while (1)
	get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated 
   with a "MODIFY:" comment.

   Do not modify the rdump and mdump functions.

   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/

/* Lab 4: exception-detect logic (datapath DP6). Combinational: recomputed at the start of
   every cycle from CURRENT values. Used by the microsequencer, cycle_memory and latch. */
int EXC = 0;          /* wire: 1 = an exception is detected this cycle (only if EXC.CHECK is set) */
int EXC_VECTOR = 0;   /* wire: vector produced by the logic: x04 unknown, x02 protection, x03 unaligned */

void eval_exception()
{
  int *mi = CURRENT_LATCHES.MICROINSTRUCTION;
  int mar = CURRENT_LATCHES.MAR;
  int op, unknown = 0, protect = 0, unaligned = 0;

  op = (CURRENT_LATCHES.IR >> 12) & 0xF;   /* opcode: (CURRENT_LATCHES.IR >> 12) & 0xF */
  if(op==10 || op==11)
  {
    unknown   = 1;   /* 1 if op is 10 or 11 (opcodes 1010, 1011) */
  }
  if(CURRENT_LATCHES.PSR15 == 1 && mar < 0x3000)
  {
    protect = 1;   /* 1 if CURRENT_LATCHES.PSR15 == 1 AND mar < 0x3000 */
  }
  if(GetDATA_SIZE(mi) == 1 && (mar & 1))
  {
    unaligned = 1;   /* 1 if GetDATA_SIZE(mi) == 1 (word) AND bit 0 of mar is 1: (mar & 1) */
  }

  EXC = 0;
  EXC_VECTOR = 0;
  if (GetEXC_CHECK(mi)) 
  {
    if (unknown)        { EXC = 1; EXC_VECTOR = 0x04;}   /* EXC = 1; EXC_VECTOR = 0x04 (highest priority) */
    else if (protect)   { EXC = 1; EXC_VECTOR = 0x02; }   /* EXC = 1; EXC_VECTOR = 0x02 (beats unaligned) */
    else if (unaligned) { EXC = 1; EXC_VECTOR = 0x03; }   /* EXC = 1; EXC_VECTOR = 0x03 */
  }
}


void eval_micro_sequencer() 
{
  /* 
   * Evaluate the address of the next state according to the 
   * micro sequencer logic. Latch the next microinstruction.
   */

  int *micro = CURRENT_LATCHES.MICROINSTRUCTION;   
  int next_state;

  eval_exception();   /* call eval_exception(): sets the EXC and EXC_VECTOR wires for this cycle */

  if (GetIRD(micro) == 1) 
  {                        
    next_state = (CURRENT_LATCHES.IR >> 12) & 0xF;                 
  }
  else 
  {
    int j    = GetJ(micro);                        
    int cond = GetCOND3(micro);   /* GetCOND3(micro): full 3-bit COND, 0-5 */
    next_state = j;

    if (cond == 1) 
    {                               
      next_state = next_state | ((1 & CURRENT_LATCHES.READY) << 1);   /* Check ready bit */
    }
    if (cond == 2) 
    {
      next_state = next_state | ((1 & CURRENT_LATCHES.BEN) << 2);   /* check branch enable */
    }
    if (cond == 3) 
    {
      next_state = next_state | ((CURRENT_LATCHES.IR >> 11) & 0x1);  /* check ir[11] */
    }
    if (cond == 4) 
    {
      next_state = next_state | ((1 & CURRENT_LATCHES.INT) << 3);   /* next_state | ((1 & CURRENT_LATCHES.INT) << 3): interrupt pending -> J[3] */
    }
    if (cond == 5) 
    {
      next_state = next_state | ((1 & CURRENT_LATCHES.PSR15) << 3);   /* next_state | ((1 & CURRENT_LATCHES.PSR15) << 3): user mode -> J[3] */
    }
  }
  if (EXC) 
  {
    next_state = 42;   /* next_state = 42: exception entry overrides every other next-state choice */
  }

  NEXT_LATCHES.STATE_NUMBER = next_state;          
  memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[next_state], sizeof(int) * CONTROL_STORE_BITS);
}


/* ---- memory model state (persists across cycles) ---- */
int mem_cycle = 0;   /* how many cycles MIO.EN has been asserted for the current access */
int MEM_OUT   = 0;   /* 16-bit word read from memory; MDR loads this in latch_datapath_values */

void cycle_memory() 
{
  int *micro = CURRENT_LATCHES.MICROINSTRUCTION;
  int  mem_en = GetMIO_EN(micro) && !EXC;   /* MEM.EN = MIO.EN AND NOT EXC: GetMIO_EN(micro) && !EXC */


  if (!mem_en) 
  {
    return;                                   /* no memory access this cycle: nothing to do */
  }

  mem_cycle = mem_cycle + 1;                     /* count this cycle of the access (becomes 1..5) */

  if (mem_cycle == 4) 
  {                                           /* 4th cycle: raise R so the sequencer sees it NEXT cycle */
    NEXT_LATCHES.READY = 1;
  }

  if (mem_cycle == 5 && CURRENT_LATCHES.READY == 1) 
  {                                           /* 5th cycle: the latched R is 1 -> perform the access now */
    int word_addr = CURRENT_LATCHES.MAR >> 1; /* MAR byte address -> MEMORY row index */
    int mar0      = CURRENT_LATCHES.MAR & 0x1;         /* MAR[0]: 0 = low byte [0], 1 = high byte [1] */
    int is_write  = GetR_W(micro);            /* 0 = RD, 1 = WR */
    int is_word   = GetDATA_SIZE(micro);      /* 0 = BYTE, 1 = WORD */

    if (is_write == 0) 
    {
      MEM_OUT = (MEMORY[word_addr][1] << 8) | MEMORY[word_addr][0];     /* build 16-bit word: high byte [1] shifted up, OR low byte [0] */
    } 
    else 
    {
      int we0 = is_word || (!is_word && mar0 == 0);     /* low byte enable: word store, OR byte store with MAR[0]==0 */
      int we1 = is_word || (!is_word && mar0 == 1);     /* high byte enable: word store, OR byte store with MAR[0]==1 */

      if (we0 == 1) 
      {
        MEMORY[word_addr][0] = CURRENT_LATCHES.MDR & 0xFF;  /* MDR[7:0] */
      }
      if (we1 == 1) 
      {
        MEMORY[word_addr][1] = (CURRENT_LATCHES.MDR >> 8) & 0xFF;  /* MDR[15:8], shifted down to 0..255 */
      }
    }

    NEXT_LATCHES.READY = 0;                /* drop R so the next access starts clean */
    mem_cycle = 0;                         /* reset the counter for the next access */
  }
}


/* ---- combinational values computed each cycle (wires, not latches) ---- */
int SR1_OUT    = 0;   /* value of the register selected by SR1MUX */
int ADDER_OUT  = 0;   /* address adder output; also used by PCMUX in latch_datapath_values */
int MARMUX_OUT = 0;   /* input to the GateMARMUX tristate */
int PC_OUT     = 0;   /* input to the GatePC tristate */

int ALU_OUT = 0;   /* input to the GateALU tristate */
int SHF_OUT = 0;   /* input to the GateSHF tristate */
int MDR_OUT = 0;   /* input to the GateMDR tristate (word, or selected+sign-extended byte) */

int SP_OUT     = 0;   /* SPMUX output: R6+2, R6-2, SSP or USP; input to the GateSP tristate */
int PSR_OUT    = 0;   /* {PSR15, 000000000000, N, Z, P}: input to the GatePSR tristate */
int VECTOR_OUT = 0;   /* x0200 + (VECTOR << 1): input to the GateVector tristate */

/* sign-extend the low 'bits' bits of value to 16 bits */
int sext(int value, int bits) 
{
  value = value & ((1 << bits) - 1);                         /* keep only the low 'bits' bits: mask = (1 << bits) - 1 */
  if (((value >> (bits - 1)) & 1) == 1) 
  {                                                 /* top bit of the field is 1 -> negative */
    value = value | ~((1<< bits) - 1);                       /* fill upper bits with 1s: OR with ~((1 << bits) - 1) */
  }
  return Low16bits(value);                                  /* return value cut to 16 bits with Low16bits(...) */
}

void eval_bus_drivers() 
{
  int *micro = CURRENT_LATCHES.MICROINSTRUCTION;
  int ir     = CURRENT_LATCHES.IR;
  int sr1_num;
  int addr1;
  int addr2;

  /* ---- SR1: which register feeds the ALU's A input, ADDR1MUX's BaseR, and the stack unit ---- */
  if (GetSR1MUX2(micro) == 0) 
  {
    sr1_num = (ir >> 9) & 0x7;     /* IR[11:9]: (ir >> 9) & 0x7 */
  } 
  else if (GetSR1MUX2(micro) == 1) 
  {
    sr1_num = (ir >> 6) & 0x7;     /* IR[8:6]: (ir >> 6) & 0x7 */
  }
  else 
  {
    sr1_num = 6;     /* 6: register R6, the stack pointer (SR1MUX = 10) */
  }
  SR1_OUT = CURRENT_LATCHES.REGS[sr1_num];                               /* CURRENT_LATCHES.REGS[sr1_num] */

  /* ---- ADDR1MUX ---- */
  if (GetADDR1MUX(micro) == 0) 
  {
    addr1 = CURRENT_LATCHES.PC;                               /* CURRENT_LATCHES.PC */
  } 
  else 
  {
    addr1 = SR1_OUT;                               /* SR1_OUT (the BaseR value) */
  }

  /* ---- ADDR2MUX ---- */
  if (GetADDR2MUX(micro) == 0) 
  {
    addr2 = 0;                                   /* 0 */
  } 
  else if (GetADDR2MUX(micro) == 1) 
  {
    addr2 = sext(ir, 6);                       /* sext(ir, 6)  -> offset6 */
  } 
  else if (GetADDR2MUX(micro) == 2) 
  {
    addr2 = sext(ir, 9);                       /* sext(ir, 9)  -> PCoffset9 */
  } 
  else 
  {
    addr2 = sext(ir, 11);                       /* sext(ir, 11) -> PCoffset11 */
  }

  /* ---- LSHF1 ---- */
  if (GetLSHF1(micro) == 1) 
  {
    addr2 = addr2 << 1;                       /* addr2 shifted left by 1 */
  }

  /* ---- address adder ---- */
  ADDER_OUT = Low16bits(addr1 + addr2);                     /* Low16bits(addr1 + addr2) */

  /* ---- MARMUX ---- */
  if (GetMARMUX(micro) == 0) 
  {
    MARMUX_OUT = ((ir & 0xFF) << 1);                  /* (ir AND 0xFF) shifted left 1  -> trap vector address */
  } 
  else 
  {
    MARMUX_OUT = ADDER_OUT;                  /* ADDER_OUT */
  }

  /* ---- PC driver ---- */
  PC_OUT = CURRENT_LATCHES.PC;                        /* CURRENT_LATCHES.PC */



    /* ---- ALU B input: imm5 or SR2 register ---- */
  int b_input;

  if (((ir >> 5) & 1) == 1) 
  {
    b_input = sext(ir, 5);                   /* sext(ir, 5) -> imm5 */
  } 
  else 
  {
    b_input = CURRENT_LATCHES.REGS[ir & 0x7];                   /* CURRENT_LATCHES.REGS[ ir AND 0x7 ] -> SR2 */
  }

  /* ---- ALU ---- */
  if (GetALUK(micro) == 0) 
  {
    ALU_OUT = Low16bits(SR1_OUT + b_input);                   /* Low16bits(SR1_OUT + b_input) */
  } 
  else if (GetALUK(micro) == 1) 
  {
    ALU_OUT = SR1_OUT & b_input;                   /* SR1_OUT AND b_input */
  } 
  else if (GetALUK(micro) == 2) 
  {
    ALU_OUT = SR1_OUT ^ b_input;                   /* SR1_OUT XOR b_input */
  } 
  else 
  {
    ALU_OUT = SR1_OUT;                   /* PASSA: just SR1_OUT */
  }

  /* ---- SHF ---- */
  int amount = ir & 0xF;                  /* IR[3:0]: ir AND 0xF */
  if (((ir >> 4) & 1) == 0) 
  {
    SHF_OUT = Low16bits(SR1_OUT << amount);                   /* LSHF: Low16bits(SR1_OUT shifted left by amount) */
  } 
  else 
  {
    SHF_OUT = SR1_OUT >> amount;                   /* RSHFL first: SR1_OUT shifted right by amount */
    if (((ir >> 5) & 1) == 1 && ((SR1_OUT >> 15) & 1) == 1) 
    {                                             /* RSHFA and original value negative */
      SHF_OUT = SHF_OUT | (((1 << amount) - 1) << (16 - amount));                 /* SHF_OUT OR (((1 << amount) - 1) << (16 - amount)) */
    }
  }

  /* ---- MDR output: whole word, or one byte sign-extended ---- */
  if (GetDATA_SIZE(micro) == 1) 
  {
    MDR_OUT = CURRENT_LATCHES.MDR;                   /* WORD: CURRENT_LATCHES.MDR */
  } 
  else 
  {
    if ((CURRENT_LATCHES.MAR & 1) == 0) 
    {
      MDR_OUT = sext(CURRENT_LATCHES.MDR & 0xFF, 8);                 /* even address: sext(MDR AND 0xFF, 8) */
    } 
    else 
    {
      MDR_OUT = sext((CURRENT_LATCHES.MDR >> 8) & 0xFF, 8);                 /* odd address: sext((MDR shifted right 8) AND 0xFF, 8) */
    }
  }

  /* ---- Lab 4: stack-pointer unit (DP3): SPMUX picks R6+2, R6-2, SSP or USP ---- */
  if (GetSPMUX(micro) == 0)      { SP_OUT = Low16bits(SR1_OUT + 2); }   /* Low16bits(SR1_OUT + 2): pop */
  else if (GetSPMUX(micro) == 1) { SP_OUT = Low16bits(SR1_OUT - 2); }   /* Low16bits(SR1_OUT - 2): push */
  else if (GetSPMUX(micro) == 2) { SP_OUT = CURRENT_LATCHES.SSP; }   /* CURRENT_LATCHES.SSP */
  else                           { SP_OUT = CURRENT_LATCHES.USP; }   /* CURRENT_LATCHES.USP */

  /* ---- Lab 4: PSR value (DP1) ---- */
  PSR_OUT = (CURRENT_LATCHES.PSR15 << 15) | (CURRENT_LATCHES.N << 2) | (CURRENT_LATCHES.Z << 1) | CURRENT_LATCHES.P;      /* (CURRENT_LATCHES.PSR15 << 15) | (CURRENT_LATCHES.N << 2) | (CURRENT_LATCHES.Z << 1) | CURRENT_LATCHES.P */

  /* ---- Lab 4: vector-table address (DP4) ---- */
  VECTOR_OUT = 0x0200 + ((CURRENT_LATCHES.VECTOR & 0xFF) << 1);   /* 0x0200 + ((CURRENT_LATCHES.VECTOR & 0xFF) << 1) */
}


void drive_bus() 
{
  int *micro = CURRENT_LATCHES.MICROINSTRUCTION;

  if (GetGATE_MARMUX(micro) == 1) 
  {
    BUS = MARMUX_OUT;        /* MARMUX_OUT */
  } 
  else if (GetGATE_PC(micro) == 1) 
  {
    BUS = PC_OUT;        /* PC_OUT */
  } 
  else if (GetGATE_ALU(micro) == 1) 
  {
    BUS = ALU_OUT;        /* ALU_OUT */
  } 
  else if (GetGATE_SHF(micro) == 1) 
  {
    BUS = SHF_OUT;        /* SHF_OUT */
  } 
  else if (GetGATE_MDR(micro) == 1) 
  {
    BUS = MDR_OUT;        /* MDR_OUT */
  } 
  else if (GetGATE_PSR(micro))    { BUS = PSR_OUT; }   /* PSR_OUT */
  else if (GetGATE_SP(micro))     { BUS = SP_OUT; }   /* SP_OUT */
  else if (GetGATE_VECTOR(micro)) { BUS = VECTOR_OUT; }   /* VECTOR_OUT */
  else 
  {
    BUS = 0;                    /* nothing gated: put 0 on the bus */
  }

  BUS = Low16bits(BUS);          /* Low16bits(BUS) so it is always 16 bits */
}


void latch_datapath_values() 
{
  int *micro = CURRENT_LATCHES.MICROINSTRUCTION;
  int ir     = CURRENT_LATCHES.IR;

  /* ---- MAR ---- */
  if (GetLD_MAR(micro) == 1) 
  {
    NEXT_LATCHES.MAR = BUS;                 /* BUS */
  }

  /* ---- MDR: from memory (5th cycle only) or from the bus ---- */
  if (GetLD_MDR(micro) == 1) 
  {
    if (GetMIO_EN(micro) == 1) 
    {
      if (CURRENT_LATCHES.READY == 1) 
      {
        NEXT_LATCHES.MDR = MEM_OUT;             /* MEM_OUT */
      }
    } 
    else 
    {
      if (GetDATA_SIZE(micro) == 1) 
      {
        NEXT_LATCHES.MDR = BUS;             /* WORD: BUS */
      } 
      else 
      {
        NEXT_LATCHES.MDR = (BUS & 0xFF) | ((BUS & 0xFF) << 8);     /* BYTE: (BUS AND 0xFF) OR ((BUS AND 0xFF) shifted left 8) */
      }
    }
  }

  /* ---- IR ---- */
  if (GetLD_IR(micro) == 1) 
  {
    NEXT_LATCHES.IR = BUS;                  /* BUS */
  }

  /* ---- BEN ---- */
  if (GetLD_BEN(micro) == 1) 
  {
    int n_bit = (ir >> 11) & 1;                        /* IR[11]: (ir shifted right 11) AND 1 */
    int z_bit = (ir >> 10) & 1;                        /* IR[10]: (ir shifted right 10) AND 1 */
    int p_bit = (ir >> 9) & 1;                         /* IR[9]:  (ir shifted right 9)  AND 1 */
    NEXT_LATCHES.BEN = (n_bit & CURRENT_LATCHES.N) | (z_bit & CURRENT_LATCHES.Z) | (p_bit & CURRENT_LATCHES.P);         /* (n_bit AND CURRENT N) OR (z_bit AND CURRENT Z) OR (p_bit AND CURRENT P) */
  }

  /* ---- register file ---- */
  if (GetLD_REG(micro) == 1) 
  {
    int dr;
    if (GetDRMUX2(micro) == 0)      { dr = (CURRENT_LATCHES.IR >> 9) & 0x7;}   /* IR[11:9]: (CURRENT_LATCHES.IR >> 9) & 0x7 */
    else if (GetDRMUX2(micro) == 1) { dr = 7; }   /* 7: R7 (JSR, JSRR, TRAP) */
    else                            { dr = 6; }   /* 6: R6, the stack pointer (DRMUX = 10) */
    NEXT_LATCHES.REGS[dr] = BUS;            /* BUS */
  }

  /* ---- condition codes: set from the value on the bus ---- */
  if (GetLD_CC(micro) == 1) 
  {
    if (((BUS >> 15) & 1) == 1) 
    {
      NEXT_LATCHES.N = 1;  NEXT_LATCHES.Z = 0;  NEXT_LATCHES.P = 0;   /* negative */
    } 
    else if (BUS == 0) 
    {
      NEXT_LATCHES.N = 0;  NEXT_LATCHES.Z = 1;  NEXT_LATCHES.P = 0;   /* zero */
    } 
    else 
    {
      NEXT_LATCHES.N = 0;  NEXT_LATCHES.Z = 0;  NEXT_LATCHES.P = 1;   /* positive */
    }
  }

  /* ---- PC ---- */
  if (GetLD_PC(micro) == 1) 
  {
    if (GetPCMUX(micro) == 0)      { NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 2); }   /* Low16bits(CURRENT_LATCHES.PC + 2) -- unchanged */
    else if (GetPCMUX(micro) == 1) { NEXT_LATCHES.PC = BUS; }   /* BUS -- unchanged */
    else if (GetPCMUX(micro) == 2) { NEXT_LATCHES.PC = ADDER_OUT; }   /* ADDER_OUT -- unchanged, was the final else */
    else                           { NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC - 2); }   /* Low16bits(CURRENT_LATCHES.PC - 2): undo the fetch increment */
  }

    /* ---- Lab 4: PSR (DP1) ---- */
  if (GetLD_PSR(micro) == 1) 
  {
    if (GetPSRMUX(micro) == 1) 
    {
      NEXT_LATCHES.PSR15 = 0;   /* 0: enter supervisor mode (entry states 41, 42) */
    } 
    else 
    {
      NEXT_LATCHES.PSR15 = (BUS >> 15) & 1;   /* (BUS >> 15) & 1: restored privilege bit (RTI, state 58) */
      NEXT_LATCHES.N     = (BUS >> 2) & 1;   /* (BUS >> 2) & 1: restored N */
      NEXT_LATCHES.Z     = (BUS >> 1) & 1;   /* (BUS >> 1) & 1: restored Z */
      NEXT_LATCHES.P     = BUS & 1;   /* BUS & 1: restored P */
    }
  }

  /* ---- Lab 4: saved stack pointers (DP3), both load from SR1_OUT (R6 when SR1MUX = 10) ---- */
  if (GetLD_SSP(micro) == 1) 
  {
    NEXT_LATCHES.SSP = SR1_OUT;   /* SR1_OUT: park the supervisor R6 (state 26) */
  }
  if (GetLD_USP(micro) == 1) 
  {
    NEXT_LATCHES.USP = SR1_OUT;   /* SR1_OUT: park the user R6 (state 44) */
  }

  /* ---- Lab 4: Vector register (DP4) and INT clear (DP6) ---- */
  if (GetLD_VECTOR(micro) == 1) 
  {
    if (GetVECTORMUX(micro) == 0) 
    {
      NEXT_LATCHES.VECTOR = CURRENT_LATCHES.INTV & 0xFF;   /* CURRENT_LATCHES.INTV & 0xFF: the timer's vector */
      NEXT_LATCHES.INT    = 0;   /* 0: the interrupt is being taken, so it's no longer pending */
    } 
    else 
    {
      NEXT_LATCHES.VECTOR = CURRENT_LATCHES.EXCV & 0xFF;   /* CURRENT_LATCHES.EXCV & 0xFF: the exception's vector */
    }
  }

  /* ---- Lab 4: EXCV register, loads whenever the exception logic fires (no control bit) ---- */
  if (EXC == 1) 
  {
    NEXT_LATCHES.EXCV = EXC_VECTOR;   /* EXC_VECTOR: from eval_exception() this cycle */
  }
}
