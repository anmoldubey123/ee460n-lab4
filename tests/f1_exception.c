/* F1 unit test: exception-detect logic (EXC, EXC_VECTOR) for each case in the design table */
#define main sim_main
#include "../lc3bsim4.c"
#undef main

void check(int state, int psr15, int mar, int ir, int exp_exc, int exp_vec) {
    CURRENT_LATCHES.PSR15 = psr15;
    CURRENT_LATCHES.MAR = mar;
    CURRENT_LATCHES.IR = ir;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[state], sizeof(int) * CONTROL_STORE_BITS);
    eval_exception();
    printf("state %2d PSR15=%d MAR=0x%04x IR=0x%04x -> EXC=%d EXCV=0x%02x  %s\n",
           state, psr15, mar, ir, EXC, EXC_VECTOR,
           (EXC == exp_exc && EXC_VECTOR == exp_vec) ? "PASS" : "FAIL");
}

int main(void) {
    init_control_store("ucode4");
    /*     state PSR15  MAR     IR      EXC  vector */
    check(23,  1, 0x0000, 0x7000,  1, 0x02);  /* STW user to system space: protection */
    check(23,  1, 0xC017, 0x7000,  1, 0x03);  /* STW user to odd address: unaligned */
    check(23,  1, 0x0001, 0x7000,  1, 0x02);  /* both: protection wins */
    check(29,  1, 0xC017, 0x2000,  0, 0x00);  /* LDB odd address: byte access is fine */
    check(24,  1, 0xC017, 0x3000,  0, 0x00);  /* STB odd address: byte access is fine */
    check(25,  1, 0x4000, 0x6000,  0, 0x00);  /* LDW user, aligned, user space: no exception */
    check(23,  0, 0x0000, 0x7000,  0, 0x00);  /* STW supervisor to x0000: allowed */
    check(10,  1, 0x3002, 0xA000,  1, 0x04);  /* opcode 1010: unknown */
    check(11,  1, 0x3002, 0xB000,  1, 0x04);  /* opcode 1011: unknown */
    check(28,  1, 0x004A, 0xF025,  0, 0x00);  /* TRAP reads x004A: no EXC.CHECK, allowed */
    return 0;
}