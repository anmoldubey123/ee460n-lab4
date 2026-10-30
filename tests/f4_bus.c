/* F4 unit test: SR1 = R6, stack unit, PSR value, vector address, and the three new gates */
#define main sim_main
#include "../lc3bsim4.c"
#undef main

int pass = 0, total = 0;

void check(char *what, int s, int exp_bus, int exp_sr1) {
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[s], sizeof(int) * CONTROL_STORE_BITS);
    eval_bus_drivers();
    drive_bus();
    int ok = (BUS == exp_bus) && (exp_sr1 < 0 || SR1_OUT == exp_sr1);
    total++; pass += ok;
    printf("state %2d %-18s BUS=0x%04x (expect 0x%04x) SR1_OUT=0x%04x  %s\n",
           s, what, BUS, exp_bus, SR1_OUT, ok ? "PASS" : "FAIL");
}

int main(void) {
    init_control_store("ucode4");
    CURRENT_LATCHES.IR = 0x8000;

    CURRENT_LATCHES.REGS[6] = 0x3000;  check("push: R6-2",  36, 0x2FFE, 0x3000);
    CURRENT_LATCHES.REGS[6] = 0x0000;  check("push wraps",  40, 0xFFFE, 0x0000);
    CURRENT_LATCHES.REGS[6] = 0x2FFC;  check("pop: R6+2",   55, 0x2FFE, 0x2FFC);
    CURRENT_LATCHES.REGS[6] = 0x2FFE;  check("last pop",    59, 0x3000, 0x2FFE);
    CURRENT_LATCHES.REGS[6] = 0x5000;  CURRENT_LATCHES.SSP = 0x3000;
                                       check("switch in",   44, 0x3000, 0x5000);
    CURRENT_LATCHES.REGS[6] = 0x3000;  CURRENT_LATCHES.USP = 0x5000;
                                       check("switch back", 26, 0x5000, 0x3000);
    CURRENT_LATCHES.PSR15 = 1; CURRENT_LATCHES.N = 0; CURRENT_LATCHES.Z = 0; CURRENT_LATCHES.P = 1;
                                       check("PSR user, P", 41, 0x8001, -1);
    CURRENT_LATCHES.PSR15 = 0; CURRENT_LATCHES.N = 1; CURRENT_LATCHES.Z = 0; CURRENT_LATCHES.P = 0;
                                       check("PSR super, N", 42, 0x0004, -1);
    CURRENT_LATCHES.VECTOR = 0x01;     check("vector timer", 47, 0x0202, -1);
    CURRENT_LATCHES.VECTOR = 0x02;     check("vector prot",  47, 0x0204, -1);
    CURRENT_LATCHES.VECTOR = 0x04;     check("vector unknown", 47, 0x0208, -1);
    CURRENT_LATCHES.REGS[6] = 0x2FFC;  check("RTI MAR<-R6",  8, 0x2FFC, 0x2FFC);
    CURRENT_LATCHES.PC = 0x3014;       check("MDR<-PC",     39, 0x3014, -1);
                                       check("no gate",     37, 0x0000, -1);
    printf("%d / %d PASS\n", pass, total);
    return 0;
}