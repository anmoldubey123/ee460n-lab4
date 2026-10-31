; Test version of add.asm (NOT submitted): identical, except the final store
; goes to xC017 (odd address, user space) to cause an unaligned access exception.
        .ORIG x3000
        LEA R0, CONSTS      ; R0 <- address of the constants table
        LDW R6, R0, #0      ; R6 <- xFE00, the user stack pointer
        LDW R1, R0, #1      ; R1 <- x4000
        AND R2, R2, #0
        ADD R2, R2, #1      ; R2 <- 1
        STW R2, R1, #0      ; M[x4000] <- 1
        LDW R1, R0, #2      ; R1 <- xC000, pointer to the first byte
        AND R3, R3, #0      ; R3 <- 0, running sum
        AND R4, R4, #0
        ADD R4, R4, #10
        ADD R4, R4, #10     ; R4 <- 20, bytes left
LOOP    LDB R5, R1, #0      ; R5 <- SEXT(M[R1]), next signed byte
        ADD R3, R3, R5      ; sum += byte
        ADD R1, R1, #1      ; advance to the next byte
        ADD R4, R4, #-1     ; one fewer byte left
        BRp LOOP
        STW R3, R1, #0      ; R1 = xC014 now: M[xC014] <- sum
        LDW R2, R0, #3      ; R2 <- xC017 (odd address)
        STW R3, R2, #0      ; unaligned access exception
        TRAP x25
CONSTS  .FILL xFE00         ; user stack pointer
        .FILL x4000         ; address of the interrupt counter
        .FILL xC000         ; address of the first data byte
        .FILL xC017         ; odd address for the unaligned store
        .END