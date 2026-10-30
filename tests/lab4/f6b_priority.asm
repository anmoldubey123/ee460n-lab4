; F6b-3: protection beats unaligned. User-mode STW to x0001 (system space AND odd).
; Expect: the PROTECTION handler (x1600) runs, not the unaligned one.
        .ORIG x3000
        LEA R0, CONST
        LDW R6, R0, #0      ; user stack pointer R6 <- xFE00
        LDW R1, R0, #1      ; R1 <- x1234
        AND R2, R2, #0
        ADD R2, R2, #1      ; R2 <- x0001, CC = P
        STW R1, R2, #0      ; both conditions true: protection must win
        ADD R3, R3, #1      ; must never execute
        TRAP x25
CONST   .FILL xFE00
VALUE   .FILL x1234
        .END