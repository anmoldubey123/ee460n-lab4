/* F2 unit test: microsequencer next-state for every Lab 4 branch, plus Lab 3 sanity cases */
#define main sim_main
#include "../lc3bsim4.c"
#undef main

int pass = 0, total = 0;

void check(int s, int ir, int mar, int psr15, int intr, int ready, int ben, int exp) {
    CURRENT_LATCHES.IR = ir;       CURRENT_LATCHES.MAR = mar;
    CURRENT_LATCHES.PSR15 = psr15; CURRENT_LATCHES.INT = intr;
    CURRENT_LATCHES.READY = ready; CURRENT_LATCHES.BEN = ben;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[s], sizeof(int) * CONTROL_STORE_BITS);
    eval_micro_sequencer();
    int ok = NEXT_LATCHES.STATE_NUMBER == exp &&
             memcmp(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[exp], sizeof(int) * CONTROL_STORE_BITS) == 0;
    total++; pass += ok;
    printf("state %2d -> %2d (expect %2d)  %s\n", s, NEXT_LATCHES.STATE_NUMBER, exp, ok ? "PASS" : "FAIL");
}

int main(void) {
    init_control_store("ucode4");
    /*    state  IR      MAR    PSR15 INT R  BEN  expect */
    check(18, 0x1000, 0x4000, 1, 0, 0, 0, 33);  /* no interrupt: normal fetch */
    check(18, 0x1000, 0x4000, 1, 1, 0, 0, 41);  /* interrupt pending: INT entry */
    check(19, 0x1000, 0x4000, 1, 1, 0, 0, 41);  /* copy of 18 */
    check(41, 0x1000, 0x4000, 1, 0, 0, 0, 44);  /* from user: switch stacks */
    check(41, 0x1000, 0x4000, 0, 0, 0, 0, 36);  /* from supervisor: push directly */
    check(42, 0x1000, 0x4000, 1, 0, 0, 0, 44);
    check(42, 0x1000, 0x4000, 0, 0, 0, 0, 36);
    check(44, 0x1000, 0x4000, 1, 0, 0, 0, 36);
    check(36, 0x1000, 0x4000, 0, 0, 0, 0, 37);
    check(37, 0x1000, 0x4000, 0, 0, 0, 0, 37);  /* write PSR: waiting */
    check(37, 0x1000, 0x4000, 0, 0, 1, 0, 39);  /* write PSR: ready */
    check(39, 0x1000, 0x4000, 0, 0, 0, 0, 40);
    check(40, 0x1000, 0x4000, 0, 0, 0, 0, 45);
    check(45, 0x1000, 0x4000, 0, 0, 1, 0, 47);
    check(47, 0x1000, 0x4000, 0, 0, 0, 0, 48);
    check(48, 0x1000, 0x4000, 0, 0, 1, 0, 50);
    check(50, 0x1000, 0x4000, 0, 0, 0, 0, 18);
    check(32, 0x8000, 0x4000, 0, 0, 0, 0,  8);  /* decode RTI */
    check( 8, 0x8000, 0x4000, 0, 0, 0, 0, 52);
    check(52, 0x8000, 0x4000, 0, 0, 1, 0, 54);
    check(54, 0x8000, 0x4000, 0, 0, 0, 0, 55);
    check(55, 0x8000, 0x4000, 0, 0, 0, 0, 56);
    check(56, 0x8000, 0x4000, 0, 0, 1, 0, 58);
    check(58, 0x8000, 0x4000, 0, 0, 0, 0, 59);
    check(59, 0x8000, 0x4000, 0, 0, 0, 0, 18);  /* restored supervisor: stay on supervisor stack */
    check(59, 0x8000, 0x4000, 1, 0, 0, 0, 26);  /* restored user: switch back */
    check(26, 0x8000, 0x4000, 1, 0, 0, 0, 18);
    check(32, 0xA000, 0x4000, 1, 0, 0, 0, 10);  /* decode opcode 1010 */
    check(10, 0xA000, 0x4000, 1, 0, 0, 0, 42);  /* unknown opcode -> exception entry */
    check(23, 0x7000, 0x0000, 1, 0, 0, 0, 42);  /* STW to x0000 in user mode -> 42 */
    check(23, 0x7000, 0x4000, 1, 0, 0, 0, 16);  /* STW to x4000 -> normal */
    check(25, 0x6000, 0x4000, 1, 0, 1, 0, 27);  /* LDW aligned, ready -> normal */
    check(25, 0x6000, 0xC017, 1, 0, 0, 0, 42);  /* LDW odd address -> 42, not 25 */
    check(29, 0x2000, 0xC017, 1, 0, 1, 0, 31);  /* LDB odd address is fine */
    check(28, 0xF025, 0x004A, 1, 0, 1, 0, 30);  /* TRAP reads x004A in user mode: allowed */
    check( 0, 0x0E00, 0x4000, 1, 0, 0, 1, 22);  /* Lab 3: BR taken (BEN -> J[2]) */
    check( 4, 0x4800, 0x4000, 1, 0, 0, 0, 21);  /* Lab 3: JSR (IR[11] -> J[0]) */
    printf("%d / %d PASS\n", pass, total);
    return 0;
}