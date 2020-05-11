\ STM8S103 Timer2 PWM
\ refer to github.com/TG9541/stm8ef/blob/master/LICENSE.md

#include STARTTEMP

\res MCU: STM8S103
\res export TIM2_PSCR TIM2_CCMR1 TIM2_CCMR2 TIM2_CCMR3
\res export TIM2_CCER1 TIM2_CCER2 TIM2_CR1 TIM2_ARRH
\res export TIM2_CCR1H TIM2_CCR2H TIM2_CCR3H

: target NVM ;
TARGET

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
    BKEY
    DUP 8 AND IF  1 FREQ +! THEN
    DUP 4 AND IF -1 FREQ +! THEN
    DUP 2 AND IF  1 DUTY +! THEN
        1 AND IF -1 DUTY +! THEN
    FREQ ? DUTY ? CR
  ;

  : init ( -- )
    [ ' UI ] LITERAL BG !
    2 T2init
  ;

 ' init 'BOOT !
ENDTEMP

\\ Example:

 15  T2PwmInit
 1000 T2Reload
 500 pwm2      \ PD3
