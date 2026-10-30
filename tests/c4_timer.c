/* C4 unit test: the timer raises INT and INTV at cycle 300 and they persist across commits */
#define main sim_main
#include "../lc3bsim4.c"
#undef main

int main(void) {
    char *args[] = {"simulate", "ucode4", "tests/lab3/full.obj"};
    initialize(args, 1);
    while (CYCLE_COUNT < 303) {
        if (CYCLE_COUNT >= 299)
            printf("start of cycle %d: INT=%d INTV=0x%02x\n",
                   CYCLE_COUNT, CURRENT_LATCHES.INT, CURRENT_LATCHES.INTV);
        cycle();
    }
    printf("after cycle 302:    INT=%d INTV=0x%02x\n", CURRENT_LATCHES.INT, CURRENT_LATCHES.INTV);
    return 0;
}