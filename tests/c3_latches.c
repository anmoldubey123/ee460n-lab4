/* C3 unit test: new latches exist and initialize() puts the machine in user mode */
#define main sim_main
#include "../lc3bsim4.c"
#undef main

void show(char *tag, System_Latches *L) {
    printf("%-8s PSR15=%d SSP=0x%04x USP=0x%04x VECTOR=0x%02x INT=%d INTV=0x%02x EXCV=0x%02x PC=0x%04x\n",
           tag, L->PSR15, L->SSP, L->USP, L->VECTOR, L->INT, L->INTV, L->EXCV, L->PC);
}

int main(void) {
    char *args[] = {"simulate", "ucode4", "tests/lab3/full.obj"};
    initialize(args, 1);                 /* 1 program file: argv[2] */
    show("CURRENT", &CURRENT_LATCHES);
    show("NEXT", &NEXT_LATCHES);
    cycle();                             /* one cycle: state 18 runs, then the commit */
    show("CYCLE 1", &CURRENT_LATCHES);
    return 0;
}