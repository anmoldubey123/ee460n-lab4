/* F5b unit test: PSR15/N/Z/P, SSP, USP, VECTOR, INT clear, EXCV */
#define main sim_main
#include "../lc3bsim4.c"
#undef main

int pass = 0, total = 0;

void step(int s) {    /* one cycle for state s: sequencer (computes EXC) -> drivers -> BUS -> latch */
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[s], sizeof(int) * CONTROL_STORE_BITS);
    NEXT_LATCHES = CURRENT_LATCHES;
    eval_micro_sequencer();
    eval_bus_drivers();
    drive_bus();
    latch_datapath_values();
}

void expect(char *what, int got, int exp) {
    total++; pass += (got == exp);
    printf("%-26s got 0x%04x expect 0x%04x  %s\n", what, got, exp, got == exp ? "PASS" : "FAIL");
}

int main(void) {
    init_control_store("ucode4");

    /* RTI restore of a user PSR with P set (x8001) */
    CURRENT_LATCHES.MDR = 0x8001;
    CURRENT_LATCHES.PSR15 = 0; CURRENT_LATCHES.N = 1; CURRENT_LATCHES.Z = 0; CURRENT_LATCHES.P = 0;
    step(58);  expect("58 x8001: PSR15", NEXT_LATCHES.PSR15, 1);
               expect("58 x8001: N",     NEXT_LATCHES.N,     0);
               expect("58 x8001: Z",     NEXT_LATCHES.Z,     0);
               expect("58 x8001: P",     NEXT_LATCHES.P,     1);

    /* RTI restore of a supervisor PSR with N set (x0004) */
    CURRENT_LATCHES.MDR = 0x0004;
    CURRENT_LATCHES.PSR15 = 1; CURRENT_LATCHES.N = 0; CURRENT_LATCHES.Z = 0; CURRENT_LATCHES.P = 1;
    step(58);  expect("58 x0004: PSR15", NEXT_LATCHES.PSR15, 0);
               expect("58 x0004: N",     NEXT_LATCHES.N,     1);
               expect("58 x0004: Z",     NEXT_LATCHES.Z,     0);
               expect("58 x0004: P",     NEXT_LATCHES.P,     0);

    /* Interrupt entry */
    CURRENT_LATCHES.PSR15 = 1; CURRENT_LATCHES.INT = 1; CURRENT_LATCHES.INTV = 0x01;
    step(41);  expect("41: PSR15 <- 0",    NEXT_LATCHES.PSR15,  0);
               expect("41: VECTOR <- INTV", NEXT_LATCHES.VECTOR, 0x01);
               expect("41: INT cleared",   NEXT_LATCHES.INT,    0);

    /* Exception entry: INT must NOT be cleared */
    CURRENT_LATCHES.PSR15 = 1; CURRENT_LATCHES.INT = 1; CURRENT_LATCHES.EXCV = 0x03;
    step(42);  expect("42: PSR15 <- 0",    NEXT_LATCHES.PSR15,  0);
               expect("42: VECTOR <- EXCV", NEXT_LATCHES.VECTOR, 0x03);
               expect("42: INT still set", NEXT_LATCHES.INT,    1);

    /* Stack switch into supervisor */
    CURRENT_LATCHES.REGS[6] = 0x5000; CURRENT_LATCHES.SSP = 0x3000; CURRENT_LATCHES.USP = 0x0000;
    step(44);  expect("44: USP <- R6",     NEXT_LATCHES.USP,     0x5000);
               expect("44: R6 <- SSP",     NEXT_LATCHES.REGS[6], 0x3000);
               expect("44: SSP unchanged", NEXT_LATCHES.SSP,     0x3000);

    /* Stack switch back to user */
    CURRENT_LATCHES.REGS[6] = 0x3000; CURRENT_LATCHES.USP = 0x5000; CURRENT_LATCHES.SSP = 0x0000;
    step(26);  expect("26: SSP <- R6",     NEXT_LATCHES.SSP,     0x3000);
               expect("26: R6 <- USP",     NEXT_LATCHES.REGS[6], 0x5000);
               expect("26: USP unchanged", NEXT_LATCHES.USP,     0x5000);

    /* EXCV loads when the exception logic fires */
    CURRENT_LATCHES.PSR15 = 1; CURRENT_LATCHES.MAR = 0xC017; CURRENT_LATCHES.IR = 0x6000; CURRENT_LATCHES.EXCV = 0;
    step(25);  expect("25 LDW xC017: EXCV", NEXT_LATCHES.EXCV, 0x03);
    CURRENT_LATCHES.MAR = 0x0000; CURRENT_LATCHES.IR = 0x7000; CURRENT_LATCHES.EXCV = 0;
    step(23);  expect("23 STW x0000: EXCV", NEXT_LATCHES.EXCV, 0x02);

    /* Lab 3 set-CC path unchanged, and PSR15 untouched by an ordinary instruction */
    CURRENT_LATCHES.IR = 0x1261; CURRENT_LATCHES.REGS[1] = 5; CURRENT_LATCHES.PSR15 = 1;
    step(1);   expect("1: ADD sets P",      NEXT_LATCHES.P,     1);
               expect("1: PSR15 untouched", NEXT_LATCHES.PSR15, 1);

    printf("%d / %d PASS\n", pass, total);
    return 0;
}