/* C2 unit test: decode ucode4 rows through the Lab 4 getters */
#define main sim_main          /* rename the simulator's main so this file can have its own */
#include "../lc3bsim4.c"
#undef main

int main(void) {
    int rows[] = {18, 23, 26, 36, 41, 42, 44, 47, 58, 59};
    int i;
    init_control_store("ucode4");
    printf("row COND SR1 DR SP EXC | LD:PSR SSP USP VEC | G:PSR SP VEC | PSRMUX VECMUX\n");
    for (i = 0; i < 10; i++) {
        int *x = CONTROL_STORE[rows[i]];
        printf("%2d   %d    %d   %d  %d  %d  |      %d   %d   %d   %d |     %d  %d   %d |      %d      %d\n",
               rows[i], GetCOND3(x), GetSR1MUX2(x), GetDRMUX2(x), GetSPMUX(x), GetEXC_CHECK(x),
               GetLD_PSR(x), GetLD_SSP(x), GetLD_USP(x), GetLD_VECTOR(x),
               GetGATE_PSR(x), GetGATE_SP(x), GetGATE_VECTOR(x),
               GetPSRMUX(x), GetVECTORMUX(x));
    }
    return 0;
}