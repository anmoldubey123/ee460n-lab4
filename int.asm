; Timer interrupt service routine: increments M[x4000], preserves every register it uses
        .ORIG x1200
        ADD R6, R6, #-2     ; push R0 on the supervisor stack
        STW R0, R6, #0
        ADD R6, R6, #-2     ; push R1
        STW R1, R6, #0
        LEA R0, ADDR        ; R0 <- address of the ADDR word
        LDW R0, R0, #0      ; R0 <- x4000
        LDW R1, R0, #0      ; R1 <- M[x4000]
        ADD R1, R1, #1      ; increment
        STW R1, R0, #0      ; M[x4000] <- M[x4000] + 1
        LDW R1, R6, #0      ; pop R1
        ADD R6, R6, #2
        LDW R0, R6, #0      ; pop R0
        ADD R6, R6, #2
        RTI                 ; pop PC and PSR, return to the interrupted program
ADDR    .FILL x4000
        .END