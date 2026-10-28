        .ORIG x3000
        LEA R0, DATA
        LDW R1, R0, #0
        LDW R2, R0, #1
        LDB R3, R0, #0
        LDB R4, R0, #1
        LDB R5, R0, #2
        LDB R6, R0, #3
        TRAP x25
DATA    .FILL x1234
        .FILL x8001
        .END