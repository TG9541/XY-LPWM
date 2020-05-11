\ STM8S103 Timer2 PWM
\ refer to github.com/TG9541/stm8ef/blob/master/LICENSE.md

#require TIM2PWM

#require WIPE
#require LED7FIRST
#require ]B!

\ EEPROM storage for frequency and decade
$4010 CONSTANT EE_FREQ
$4012 CONSTANT EE_DECA
$4014 CONSTANT EE_DUTY

NVM
  VARIABLE STOR   \ neg. number starts EEPROM store sequence
  VARIABLE UPDA   \ resquest LCD update
  VARIABLE INCR   \ progressive increment/decrement

  : UI ( -- )
    \ key release resets the progressive key-hold increment
    BKEY 0= IF
      1 INCR !
    THEN

    \ show symbol SET while/after keys are pressed, store values to EEPROM
    STOR @ 0< IF
      1 STOR +!
      STOR @ IF
        [ 1 LCDSYM 1+ 5 ]B!
      ELSE
        ULOCK
        FREQ @ EE_FREQ !
        DECA @ EE_DECA !
        DUTY @ EE_DUTY !
        LOCK
        [ 0 LCDSYM 1+ 5 ]B!
        1 UPDA !
      THEN
    THEN

    \ UPDA request LCD refresh, ?KEY produces ASCII codes with auto-repeat
    UPDA @ IF  CR  0 DUP UPDA !  0 1 ELSE  ?KEY  THEN IF
      INCR @ ( c inc )
      OVER 72 ( "H" ) = IF DUP FREQ @  + 1000 MIN FREQ ! NIP 237 SWAP THEN
      OVER 68 ( "D" ) = IF FREQ @ OVER -    0 MAX FREQ ! NIP 237 SWAP THEN
      OVER 66 ( "B" ) = IF DUP DUTY @  +   99 MIN DUTY ! NIP  17 SWAP THEN
      OVER 65 ( "A" ) = IF DUTY @ OVER -    1 MAX DUTY ! NIP  17 SWAP THEN

      \ detect a keypress (c=0 signals LCD update)
      OVER ( c/lim ) IF
        -100 STOR !
      THEN

      ( c/lim inc) 3 + MIN INCR !

      FREQ @ 1000 = IF
         1 DECA +!
        100 FREQ !
      THEN
      FREQ @ 100 < DECA @ AND IF
        -1 DECA +!
        999 FREQ !
      THEN

      DECA @
      DUP 3 = IF
        \ limit the 3rd decade to 150k
        FREQ @ DUP 151 < NOT IF
          DROP 150
        THEN
        DUP FREQ !  10 * .
        \ decimal point in decade "3"
        [ 1 LED7FIRST 2 + 7 ]B!
      ELSE
        \ lower limit of frequency in decade "0"
        DUP 0 = FREQ @ 1 < AND IF 1 FREQ ! THEN
        FREQ ?
        \ decimal points in decades "1" and "2"
        DUP 1 = IF [ 1 LED7FIRST 1 + 7 ]B! THEN
        DUP 2 = IF [ 1 LED7FIRST 2 + 7 ]B! THEN
      THEN

      ( deca ) DROP DUTY ? CR
      CALCPWM
    THEN
  ;

  : init ( -- )
    ULOCK
    \ initialyze frequency and decade, initualize if out-of-range
    EE_DECA @ DUP 0    4 WITHIN NOT IF DROP   0 DUP EE_DECA ! THEN DECA !
    EE_FREQ @ DUP 1 1000 WITHIN NOT IF DROP 100 DUP EE_FREQ ! THEN FREQ !
    EE_DUTY @ DUP 1  100 WITHIN NOT IF DROP  50 DUP EE_DUTY ! THEN DUTY !
    LOCK

    4 LCDSYM !
    1   UPDA !
    [ ' UI ] LITERAL BG !
    TIM2PWM
    hi
  ;

 ' init 'BOOT !
WIPE RAM

\\ Example:

 15  T2PwmInit
 1000 T2Reload
 500 pwm2      \ PD3
