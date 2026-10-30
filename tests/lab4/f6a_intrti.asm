; F6a: timer interrupt + RTI back to user mode.
; Loop counts R1 up from -100 to 0 with BRn; R2 counts iterations.
; The BRnzp below is timing padding: it makes the cycle-300 interrupt land
; right before BRn, so BRn depends on the N bit that RTI restores.
; If RTI does not restore N (the ISR leaves P set), BRn falls through and R2 < 100.
        .ORIG x3000
        LEA R0, CONST
        LDW R6, R0, #0      ; user stack pointer R6 <- xFE00
        LDW R1, R0, #1      ; R1 <- -100
        AND R2, R2, #0      ; R2 <- 0
        BRnzp LOOP          ; padding only (always taken, 10 cycles)
LOOP    ADD R2, R2, #1      ; one more iteration
        ADD R1, R1, #1      ; R1 <- R1 + 1 (N while negative)
        BRn LOOP
        TRAP x25
CONST   .FILL xFE00
COUNT   .FILL #-100
        .END