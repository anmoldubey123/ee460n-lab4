; F6b-1: protection exception. User-mode STW to x0000 (system space).
; Expect: protection handler (x1600) runs, x0000 is NOT written,
; pushed PC = address of the STW, R3 stays 0 (the ADD after the STW never runs).
        .ORIG x3000
        LEA R0, CONST
        LDW R6, R0, #0      ; user stack pointer R6 <- xFE00
        LDW R1, R0, #1      ; R1 <- x1234, the value we try to store
        AND R2, R2, #0      ; R2 <- x0000 (system space), CC = Z
        STW R1, R2, #0      ; protection exception
        ADD R3, R3, #1      ; must never execute
        TRAP x25
CONST   .FILL xFE00
VALUE   .FILL x1234
        .END