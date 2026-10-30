/* F5a unit test: DRMUX = R6 and PCMUX = PC-2 in latch_datapath_values, plus Lab 3 sanity */
#define main sim_main
#include "../lc3bsim4.c"
#undef main

int pass = 0, total = 0;

void step(int s) {    /* one datapath cycle for state s: drivers -> BUS -> latch into NEXT */
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[s], sizeof(int) * CONTROL_STORE_BITS);
    NEXT_LATCHES = CURRENT_LATCHES;
    eval_bus_drivers();
    drive_bus();
    latch_datapath_values();
}

void expect(char *what, int got, int exp) {
    total++; pass += (got == exp);
    printf("%-22s got 0x%04x expect 0x%04x  %s\n", what, got, exp, got == exp ? "PASS" : "FAIL");
}

int main(void) {
    init_control_store("ucode4");

    CURRENT_LATCHES.REGS[6] = 0x3000;
    step(36);  expect("36: R6 <- R6-2",  NEXT_LATCHES.REGS[6], 0x2FFE);
              expect("36: MAR <- R6-2", NEXT_LATCHES.MAR,     0x2FFE);

    CURRENT_LATCHES.REGS[6] = 0x5000;  CURRENT_LATCHES.SSP = 0x3000;
    step(44);  expect("44: R6 <- SSP",   NEXT_LATCHES.REGS[6], 0x3000);

    CURRENT_LATCHES.REGS[6] = 0x2FFE;
    step(59);  expect("59: R6 <- R6+2",  NEXT_LATCHES.REGS[6], 0x3000);

    CURRENT_LATCHES.PC = 0x3016;  CURRENT_LATCHES.PSR15 = 1;
    CURRENT_LATCHES.N = 0;  CURRENT_LATCHES.Z = 0;  CURRENT_LATCHES.P = 1;
    step(41);  expect("41: PC <- PC-2",  NEXT_LATCHES.PC,  0x3014);
              expect("41: MDR <- PSR",  NEXT_LATCHES.MDR, 0x8001);

    CURRENT_LATCHES.PC = 0x0000;
    step(41);  expect("41: PC-2 wraps",  NEXT_LATCHES.PC,  0xFFFE);

    CURRENT_LATCHES.MDR = 0x1200;
    step(50);  expect("50: PC <- MDR",   NEXT_LATCHES.PC,  0x1200);

    CURRENT_LATCHES.IR = 0x1261;  CURRENT_LATCHES.REGS[1] = 5;  CURRENT_LATCHES.REGS[6] = 0x3000;
    step(1);   expect("1: ADD R1,R1,#1", NEXT_LATCHES.REGS[1], 0x0006);
              expect("1: R6 untouched", NEXT_LATCHES.REGS[6], 0x3000);

    CURRENT_LATCHES.IR = 0x4802;  CURRENT_LATCHES.PC = 0x3010;
    step(21);  expect("21: JSR R7 <- PC", NEXT_LATCHES.REGS[7], 0x3010);
              expect("21: JSR PC+off",   NEXT_LATCHES.PC,      0x3014);

    printf("%d / %d PASS\n", pass, total);
    return 0;
}