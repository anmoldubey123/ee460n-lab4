; F6b-4: unknown opcode exception. Executes the word xA000 (opcode 1010).
; Expect: unknown-opcode handler (x1C00) runs, pushed PC = address of the xA000 word.
        .ORIG x3000
        LEA R0, CONST
        LDW R6, R0, #0      ; user stack pointer R6 <- xFE00
        LDW R1, R0, #1      ; R1 <- x1234, CC = P
        .FILL xA000         ; opcode 1010: unknown opcode exception
        ADD R3, R3, #1      ; must never execute
        TRAP x25
CONST   .FILL xFE00
VALUE   .FILL x1234
        .END