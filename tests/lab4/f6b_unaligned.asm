; F6b-2: unaligned access exception. User-mode STW to xC017 (odd address).
; Expect: unaligned handler (x1A00) runs, xC016 is NOT written,
; pushed PC = address of the STW, R3 stays 0.
        .ORIG x3000
        LEA R0, CONST
        LDW R6, R0, #0      ; user stack pointer R6 <- xFE00
        LDW R1, R0, #1      ; R1 <- x1234
        LDW R2, R0, #2      ; R2 <- xC017 (odd), CC = N
        STW R1, R2, #0      ; unaligned access exception
        ADD R3, R3, #1      ; must never execute
        TRAP x25
CONST   .FILL xFE00
VALUE   .FILL x1234
ODD     .FILL xC017
        .END