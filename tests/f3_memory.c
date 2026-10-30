/* F3 unit test: MEM.EN = MIO.EN AND NOT EXC */
#define main sim_main
#include "../lc3bsim4.c"
#undef main

int main(void) {
    int k;
    init_control_store("ucode4");

    /* Case 1: user-mode LDW from xC017 (unaligned) -> EXC = 1, memory must not start */
    MEMORY[0xC016 >> 1][0] = 0xCD;  MEMORY[0xC016 >> 1][1] = 0xAB;
    mem_cycle = 0;  MEM_OUT = 0x1234;                 /* 0x1234 = "untouched" marker */
    CURRENT_LATCHES.PSR15 = 1;   CURRENT_LATCHES.MAR = 0xC017;
    CURRENT_LATCHES.IR = 0x6000; CURRENT_LATCHES.READY = 0;  NEXT_LATCHES.READY = 0;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[25], sizeof(int) * CONTROL_STORE_BITS);
    eval_micro_sequencer();
    cycle_memory();
    printf("faulting LDW: EXC=%d next=%d mem_cycle=%d READY=%d MEM_OUT=0x%04x\n",
           EXC, NEXT_LATCHES.STATE_NUMBER, mem_cycle, NEXT_LATCHES.READY, MEM_OUT);

    /* Case 2: user-mode LDW from x4000 (legal) -> normal 5-cycle read of x5678 */
    MEMORY[0x4000 >> 1][0] = 0x78;  MEMORY[0x4000 >> 1][1] = 0x56;
    CURRENT_LATCHES.MAR = 0x4000;
    for (k = 1; k <= 5; k++) {
        eval_micro_sequencer();
        cycle_memory();
        printf("legal LDW cycle %d: EXC=%d mem_cycle=%d READY(next)=%d MEM_OUT=0x%04x\n",
               k, EXC, mem_cycle, NEXT_LATCHES.READY, MEM_OUT);
        CURRENT_LATCHES.READY = NEXT_LATCHES.READY;   /* the READY commit the shell would do */
    }
    return 0;
}