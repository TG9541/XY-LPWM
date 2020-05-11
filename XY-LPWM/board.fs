\ STM8S103 Timer2 PWM
\ refer to github.com/TG9541/stm8ef/blob/master/LICENSE.md

#include STARTTEMP

\res MCU: STM8S103
\res export TIM2_PSCR TIM2_CCMR1 TIM2_CCMR2 TIM2_CCMR3
\res export TIM2_CCER1 TIM2_CCER2 TIM2_CR1 TIM2_ARRH
\res export TIM2_CCR1H TIM2_CCR2H TIM2_CCR3H

#require LED7FIRST
#require ]B!

: TARGET NVM ;

TARGET

VARIABLE UPDA
VARIABLE INCR
VARIABLE DECA
VARIABLE FREQ
VARIABLE DUTY

  \
  : T2Pre ( n -- )
    ( n ) TIM2_PSCR  C!  \ TIMx prescaler (2^n)
  ;

  \ Set Timer2 reload value
  : T2Reload ( n -- )
    TIM2_ARRH 2C!
  ;

  \ Set PWM2 compare value
  : PWM2 ( n -- )
    TIM2_CCR2H 2C!
  ;

  \ convert duty cycle [1/1000] to PWM reload value
  : setduty ( n -- n )
    TIM2_ARRH 2C@ 1000 */
  ;


  \ Init Timer2 with prescaler CC PWM2
  : T2init ( n -- )
    ( n ) T2Pre
    $60 TIM2_CCMR2 C!    \ TIMx OC2M = PWM mode 1
    $10 TIM2_CCER1 C!    \ TIMx CC2 enable
    1   TIM2_CR1 C!
  ;

  : UI ( -- )
    \ key release resets the progressive key-hold increment
    BKEY 0= IF
      1 INCR !
    THEN

    \ STM8 eForth board keys with auto-repeat use ASCII codes
    UPDA @ IF  CR  0 DUP UPDA !  0 1 ELSE  ?KEY  THEN IF
      INCR @ ( c inc )
      OVER 72 ( "H" ) = IF DUP FREQ @  + 1000 MIN FREQ ! 237 THEN
      OVER 68 ( "D" ) = IF FREQ @ OVER -    0 MAX FREQ ! 237 THEN
      OVER 66 ( "B" ) = IF DUP DUTY @  +   99 MIN DUTY !  17 THEN
      OVER 65 ( "A" ) = IF DUTY @ OVER -    1 MAX DUTY !  17 THEN
      ( c inc lim ) SWAP 3 + MIN INCR ! ( c ) DROP

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

    THEN
  ;

  : init ( -- )
      4 LCDSYM !
      1   DECA !
    500   FREQ !
      1   UPDA !
    [ ' UI ] LITERAL BG !
    2 T2init
  ;

 ' init 'BOOT !
ENDTEMP

\\ Example:

 15  T2PwmInit
 1000 T2Reload
 500 pwm2      \ PD3
