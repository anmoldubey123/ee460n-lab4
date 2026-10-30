; Interrupt/exception vector table: entry for vector V is at x0200 + 2V
        .ORIG x0200
        .FILL x0000     ; x0200: vector x00 (unused)
        .FILL x1200     ; x0202: vector x01, timer interrupt  -> ISR
        .FILL x1600     ; x0204: vector x02, protection       -> handler
        .FILL x1A00     ; x0206: vector x03, unaligned access -> handler
        .FILL x1C00     ; x0208: vector x04, unknown opcode   -> handler
        .END