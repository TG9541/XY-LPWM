; STM8S003F3P6 "" STM8S device dependent routine default code

;===============================================================

;      Dummy labels for PSIM interrupts declared in main.c

        .ifne   PSIM-PORTA
;       Dummy label for _EXTIA_IRQHandler
_EXTI0_IRQHandler:
        .endif

        .ifne   PSIM-PORTB
;       Dummy label for _EXTIB_IRQHandler
_EXTI1_IRQHandler:
        .endif

        .ifne   PSIM-PORTC
;       Dummy label for _EXTIC_IRQHandler
_EXTI2_IRQHandler:
        .endif

        .ifne   PSIM-PORTD
;       Dummy label for _EXTID_IRQHandler
_EXTI3_IRQHandler:
        .endif

        .ifne   PSIM-PORTE
;       Dummy label for _EXTIE_IRQHandler
_EXTI4_IRQHandler:
        .endif

;===============================================================

;       BOARDINIT  ( -- )
;       Init board GPIO (except COM ports)

BOARDINIT:
        ; Board I/O initialization
        MOV     PA_DDR, #0b00000000 ; All as input
        MOV     PC_DDR, #0b00000000 ; All as input
        ; Inputs:Duty+ [PA1], Duty- [PA2]
        MOV     PA_ODR, #0b00000110
        MOV     PA_CR1, #0b00000110 ; Inputs with pullup
        MOV     PA_CR2, #0b00000000 ; No external interrupt
        ; Inputs: Freq+ [PC4], Freq- [PC3]
        ; Outputs: HT1621: DATA [PC7], CLK(nWR) [PC6], nCS [PC5]
        MOV     PC_ODR, #0b11100000
        MOV     PC_CR1, #0b11111000 ; push-pull outputs, pullup inputs
        MOV     PC_CR2, #0b11100000
        MOV     PC_DDR, #0b11100000 ; HT1621B pins is output

        ; Init HT1621B
        LD      A, #0x28 ; Bias 1/2, 4 commons
        CALLR   LCD_CMD
        LD      A, #0x18 ; RC 256KHz
        CALLR   LCD_CMD
        LD      A, #0x00 ; Sys disable
        CALLR   LCD_CMD
        LD      A, #0x05 ; WDT disable
        CALLR   LCD_CMD
        LD      A, #0x01 ; System enable
        CALLR   LCD_CMD
        LD      A, #0x03 ; LCD on
        JRA     LCD_CMD

; ==============================================

; Send command to LCD. reg A - command
LCD_CMD:
        BRES    PC_ODR, #5 ; CS active
        CALLR   LCD_BIT1
        CALLR   LCD_BIT0
        CALLR   LCD_BIT0
        ; Send command
        CALLR   LCD_BYTE_A
        CALLR   LCD_BIT0
        BSET    PC_ODR, #5 ; CS inactive
        RET

; Send address to LCD
LCD_ADDR:
        BRES    PC_ODR, #5 ; CS active
        CALLR   LCD_BIT1
        CALLR   LCD_BIT0
        CALLR   LCD_BIT1
        ; Send addr
        RLC     A
        RLC     A
        CALLR   LCD_BIT_A
        CALLR   LCD_BIT_A
        CALLR   LCD_BIT_A
        CALLR   LCD_BIT_A
        CALLR   LCD_BIT_A
        CALLR   LCD_BIT_A
        RET

; Cascade through bit/nibble/byte transfers
LCD_BIT1:
        BSET    PC_ODR, #7 ; Data 1
        JRA     LCD_CLK

LCD_BIT0:
        BRES    PC_ODR, #7 ; Data 0
        JRA     LCD_CLK

; byte transfer with LED->LCD pattern mapping
LCD_BYTE_M:
        CALLR   LCD_MAP
        JRA     LCD_BYTE_A

LCD_BYTE_SWAP_M:
        CALLR   LCD_MAP
        ; fall through

; @plumbum 's compact bit-bang routines
LCD_BYTE_SWAP_A:
        SWAP    A
LCD_BYTE_A:
        CALLR   LCD_BIT_A ; bit 7
        CALLR   LCD_BIT_A
        CALLR   LCD_BIT_A
        CALLR   LCD_BIT_A ; bit 4
LCD_NIBBLE_A:
        CALLR   LCD_BIT_A ; bit 3
        CALLR   LCD_BIT_A
        CALLR   LCD_BIT_A ; bit 1
        ; bit 0
LCD_BIT_A:
        RLC     A
        BCCM    PC_ODR, #7 ; Data
LCD_CLK:
        BRES    PC_ODR, #6 ; Clock down
        BSET    PC_ODR, #6 ; Clock up
        RET

;       >LCD  ( -- )
;       Update LCD data.
;       Copy 9 bytes from LED memory to LCD data registers.
;       Bytes 0-3 mapped to 4 digits, dots and sign V (3 bit0) on upper row.
;       Bytes 4-7 mapped to 4 digits, dots and semicolon (7 bit0) on lower row.
;       LCDSYM mapped to symbols ( :, V, IN, OUT, SET, °C, W, %, h, A )
        .dw     LINK
        LINK =  .
        .db     (4)
        .ascii  ">LCD"
LCD_UP:
        BTJF    LCDSYM,#0,1$     ; symbol "V"
1$:     BCCM    LED7FIRST+3,#7   ; MSB of lower row rightmost digit
        BTJF    LCDSYM,#1,2$     ; symbol ":"
2$:     BCCM    LED7FIRST+7,#7   ; MSB of upper row rightmost digit

        CLR     A
        CALLR   LCD_ADDR
        ; Lower row
        LD      A, LED7FIRST+4
        CALLR   LCD_BYTE_M
        LD      A, LED7FIRST+5
        CALLR   LCD_BYTE_M
        LD      A, LED7FIRST+6
        CALLR   LCD_BYTE_M
        LD      A, LED7FIRST+7
        CALLR   LCD_BYTE_M
        ; Signs ( W, %, h, A )
        LD      A, LCDSYM+1
        SWAP    A
        CALLR   LCD_NIBBLE_A

        ; Upper row
        LD      A, LED7FIRST+3
        CALLR   LCD_BYTE_SWAP_M
        LD      A, LED7FIRST+2
        CALLR   LCD_BYTE_SWAP_M
        LD      A, LED7FIRST+1
        CALLR   LCD_BYTE_SWAP_M
        LD      A, LED7FIRST
        CALLR   LCD_BYTE_SWAP_M
        ; Signs ( IN, OUT, SET, °C )
        LD      A, LCDSYM+1
        CALLR   LCD_NIBBLE_A

        BSET    PC_ODR, #5 ; CS inactive
        RET

; map STM8 eForth 7S LED bit pattern to @plumbum 's solution
        RamWord LCDCONV          ; we need some RAM for bit operations
LCD_MAP:
        LD      LCDCONV,A        ; pGFEDCBA
        BTJF    LCDCONV,#7,1$    ; p->C
1$:     RLC     A
        AND     A,#0b01110001
        LD      LCDCONV+1,A      ; .FED...p
        BTJF    LCDCONV,#0,2$    ; A->C
2$:     LD      A,LCDCONV
        RRC     A                ; ApGFEDCB
        AND     A,#0b10000010
        OR      A,LCDCONV+1
        LD      LCDCONV+1,A      ; AFED..Cp
        BTJF    LCDCONV,#6,3$    ; G->C
3$:     BCCM    LCDCONV+1,#2     ; AFED.GCp
        BTJF    LCDCONV,#1,4$    ; B->C
4$:     BCCM    LCDCONV+1,#3     ; AFEDBGCp
        LD      A,LCDCONV+1
        RET

; ==============================================

;       LCDSYM  ( -- a )
;       Variable for LCD symbols bitmap
;       Symbols :, V, IN, OUT, SET, °C, W, %, h, A
        RamWord LCDSYM
        .dw     LINK
        LINK =  .
        .db     (6)
        .ascii  "LCDSYM"
LCD_SYM:
        LD      A,#LCDSYM
        JP      ASTOR


; ==============================================

        .ifne   HAS_LED7SEG
;       LED_MPX driver ( -- )
;       Code called from ISR for LED MPX

LED_MPX:
        ; updating the LCD takes 95µs
        JP    LCD_UP
        .endif

; ==============================================

        .ifne   HAS_OUTPUTS
;       OUT!  ( c -- )
;       Put c to board outputs, storing a copy in OUTPUTS
        .dw     LINK

        LINK =  .
        .db     (4)
        .ascii  "OUT!"
OUTSTOR:
        RET
        .endif

;===============================================================

        .ifne   HAS_KEYS
;       BKEY  ( -- c )     ( TOS STM8: -- A,Z,N )
;       Read board keys state as a bitfield.
;       Bits:
;           bit 3: Freq+ [PC4]
;           bit 2: Freq- [PC3]
;           bit 1: Duty+ [PA1]
;           bit 0: Duty- [PA2]
        .dw     LINK

        LINK =  .
        .db     (4)
        .ascii  "BKEY"
BKEY:
        CLR     A
        BTJF    PC_IDR, #4, 1$
1$:     RLC     A
        BTJF    PC_IDR, #3, 2$
2$:     RLC     A
        BTJF    PA_IDR, #1, 3$
3$:     RLC     A
        BTJF    PA_IDR, #2, 4$
4$:     RLC     A
        CPL     A
        AND     A, #0x0F
        JP      ASTOR


;       BKEYC  (  -- c )   ( TOS STM8: -- A,Z,N )
;       Read and translate board dependent key bitmap into char

BKEYCHAR:
        CALL    BKEY
        JREQ    1$
        ADD     A,#'@'
        LD      (1,X),A
1$:     RET
       .endif

; ==============================================
; @plumbum 's solution for LCD access and development words

; For reference: @plumbum 's LCD_FONT (replaced by remapping the STM8 eForth LED patterns)
; LCD_FONT:
; 2.             !    "    #    $    %    &    '    (    )    *    +    ,    -    .    /
;        .db      0x60,0x48,0x7E,0xD6,0x46,0xFC,0x08,0xF0,0x9A,0xCC,0x0c,0x02,0x04,0x02,0x28
; 3.        0    1    2    3    4    5    6    7    8    9    :    ;    <    =    >    ?
;        .db 0xFA,0x0A,0xBC,0x9E,0x4E,0xD6,0xF6,0x8A,0xFE,0xDE,0x00,0x00,0x30,0x14,0x88,0xAC
; 4.        @    A    B    C    D    E    F    G    H    I    J    K    L    M    N    O
;        .db 0xBE,0xEE,0x76,0x34,0x3E,0xF4,0xE4,0xF2,0x66,0x60,0x1A,0x7C,0x70,0xA6,0x26,0x36
; 5.        P    Q    R    S    T    U    V    W    X    Y    Z    [    \    ]    ^    _
;        .db 0xEC,0xCE,0x24,0xD6,0x74,0x7A,0x32,0xB2,0x6E,0x5e,0xBC,0xF0,0x42,0x9A,0xC8,0x10
; 6.        `    a    b    c    d    e    f    g    h    i    j    k    l    m    n    o
; 7.        p    q    r    s    t    u    v    w    x    y    z    {    |    }    ~    DEL

; HT1621B segments map
; addr  b3      b2      b1      b0
; -------------------------------------
;  0    2.1A    2.1F    2.1E    2.1D
;  1    2.1B    2.1G    2.1C    2.1dot
;  2    2.2A    2.2F    2.2E    2.2D
;  3    2.2B    2.2G    2.2C    2.2dot
;  4    2.3A    2.3F    2.3E    2.3D
;  5    2.3B    2.3G    2.3C    2.3dot
;  6    2.4A    2.4F    2.4E    2.4D
;  7    2.4B    2.4G    2.4C    2.colon
;  8    2.W     2.%     2.h     2.A(mp)
;  9    1.4B    1.4G    1.4C    1.V
; 10    1.4A    1.4F    1.4E    1.4D
; 11    1.3B    1.3G    1.3C    1.3dot
; 12    1.3A    1.3F    1.3E    1.3D
; 13    1.2B    1.2G    1.2C    1.2dot
; 14    1.2A    1.2F    1.2E    1.2D
; 15    1.1B    1.1G    1.1C    1.1dot
; 16    1.1A    1.1F    1.1E    1.1D
; 17    1.IN    1.OUT   2.SET   1.°C


; @plumbum 's alphanum codes
; 0 - $FA ; 1 - $0A ; 2 - $BC ; 3 - $9E ; 4 - $4E ; 5 - $D6 ; 6 - $F6 ; 7 - $8A
; 8 - $FE ; 9 - $DE ; A - $EE ; b - $76 ; C - $F0 ; c - $34 ; d - $3E ; E - $F4
; F - $E4 ; G - $F2 ; h - $66 ; I - $60 ; J - $1A ; k - $7c ; L - $70 ; M - $A6
; n - $26 ; o - $36 ; P - $EC ; Q - $CE ; r - $24 ; s - $D6 ; t - $74 ; U - $7A
; v - $32 ; w - $B2 ; X - $6E ; y - $5e ; Z - $BC ; - - $04 ; ^ - $C8 ; ° - $CC
; _ - $10

        .ifne   LCD_DIRECT

; ==============================================
;       'LCD  ( -- ref )
;       Get address of LCD double buffer
        RamBlck LCD_BUF,1
        .dw     LINK

        LINK =  .
        .db     (4)
        .ascii  "'LCD"
LCD7FIRST:
        LDW     Y,#(LCD_BUF)
        JP      YSTOR

; ==============================================
;       C7S  ( c -- c7s )
;       Convert char to 7 seg code
        .dw     LINK

        LINK =  .
        .db     (3)
        .ascii  "C7S"
C7S:
        ; Load char
        LD      A, (#1, X)
        CP      A, #0x7F ; A <= $7F
        JRULE   1$
        SLL     A
        JRA     3$
1$:
        CP      A, #0x5F ; A <= $5F
        JRULE   2$
        SUB     A, #0x20
2$:
        SUB     A, #0x21
3$:
        CLRW    Y
        LD      YL, A
        LD      A, (#LCD_FONT, Y)
        LD      (#1, X), A
        CLR     A
        LD      (X), A
        RET

;       LCD!  ( w addr -- )
;       Put w (4 nibbles) to LCD data registers
        .dw     LINK

        LINK =  .
        .db     (4)
        .ascii  "LCD!"
LCD_STOR:
        ; Load address
        INCW    X ; Skip msb byte
        LD      A,(X) ; Get lsb byte
        INCW    X
        ; Load data
        LDW     Y, X
        LDW     Y, (Y)
        INCW    X
        INCW    X
        ; Load values and now write to HT1621B

LCD_DATA:
        CALLR   LCD_ADDR
        ; Send data
        LD      A,YH
        CALLR   LCD_BYTE_A
        LD      A,YL
        CALLR   LCD_BYTE_A

        BSET    PC_ODR, #5 ; CS inactive
        RET

; ==============================================
;       LCDC!  ( c addr -- )
;       Put c (2 nibbles) to LCD data registers
        .dw     LINK

        LINK =  .
        .db     (5)
        .ascii  "LCDC!"
LCDC_STOR:
        ; Load address
        INCW    X ; Skip msb byte
        LD      A,(X) ; Get lsb byte
        INCW    X
        ; Load data
        LDW     Y, X
        LDW     Y, (Y)
        INCW    X
        INCW    X
        ; Load values and now write to HT1621B
LCDC_DATA:
        CALLR   LCD_ADDR
        ; Send data
        LD      A,YL
        CALLR   LCD_BYTE_A

        BSET    PC_ODR, #5 ; CS inactive
        RET

; ==============================================
;       >LCD  ( ref -- )
;       Update LCD data.
;       Copy 9 bytes from memory ref addr to LCD data registers.
;       Bytes 0-3 mapped to 4 digits, dots and sign V (3 bit0) on upper row.
;       Bytes 4-7 mapped to 4 digits, dots and semicolon (7 bit0) on lower row.
;       Byte  8 mapped to signs ( IN, OUT, SET, °C, W, %, h, A )
        .dw     LINK

        LINK =  .
        .db     (4)
        .ascii  ">LCD"
LCD_UP:
        ; Load address
        LDW     Y,X
        LDW     Y,(Y)
        INCW    X
        INCW    X
_LCD_UP:
        ; Remap to LCD
        CLR     A
        CALLR   LCD_ADDR
        ; Lower row
        LD      A, (#4, Y)
        CALLR   LCD_BYTE_A
        LD      A, (#5, Y)
        CALLR   LCD_BYTE_A
        LD      A, (#6, Y)
        CALLR   LCD_BYTE_A
        LD      A, (#7, Y)
        CALLR   LCD_BYTE_A
        ; Signs ( W, %, h, A )
        LD      A, (#8, Y)
        SWAP    A
        CALLR   LCD_NIBBLE_A

        ; Upper row
        LD      A, (#3, Y)
        CALLR   LCD_BYTE_SWAP_A
        LD      A, (#2, Y)
        CALLR   LCD_BYTE_SWAP_A
        LD      A, (#1, Y)
        CALLR   LCD_BYTE_SWAP_A
        LD      A, (Y)
        CALLR   LCD_BYTE_SWAP_A
        ; Signs ( IN, OUT, SET, °C )
        LD      A, (#8, Y)
        CALLR   LCD_NIBBLE_A

        BSET    PC_ODR, #5 ; CS inactive
        RET


        ; LCD_DIRECT

;===============================================================
; @plumbum 's  development words

;       WRST  ( -- )
;       Emulate software reset trought WWDG
        .dw     LINK

        LINK =  .
        .db     (4)
        .ascii  "WRST"
WRST:
        MOV     WWDG_CR, #0xC0
WRSTLOOP:
        JRA     WRSTLOOP

;===============================================================
;       SWAPN  ( c -- c )     ( TOS STM8: -- A,Z,N )
;       swap nibbles in byte
        .dw     LINK

        LINK =  .
        .db     (5)
        .ascii  "SWAPN"
SWAPN:
        SWAP    (1,X)
        RET

;===============================================================
;       SWAPW  ( w -- w )     ( TOS STM8: -- A,Z,N )
;       swap bytes in word
        .dw     LINK

        LINK =  .
        .db     (5)
        .ascii  "SWAPW"
SWAPW:
        LDW     Y,X
        LDW     Y,(Y)
        SWAPW   Y
        LDW     (X),Y
        RET

       .endif

