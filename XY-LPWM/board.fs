\ STM8S103 Timer2 PWM
\ refer to github.com/TG9541/stm8ef/blob/master/LICENSE.md

#include STARTTEMP

\res MCU: STM8S103
\res export TIM2_PSCR
\res export TIM2_CCMR1
\res export TIM2_CCMR2
\res export TIM2_CCMR3
\res export TIM2_CCER1
\res export TIM2_CCER2
\res export TIM2_CR1
\res export TIM2_ARRH
\res export TIM2_CCR1H
\res export TIM2_CCR2H
\res export TIM2_CCR3H

TARGET

  \ Init Timer2 with prescaler ( n=15 -> 1 MHz), CC PWM1..PWM3
  : T2PwmInit ( n -- )
    ( n ) TIM2_PSCR  C!  \ TIM2,3,5 prescaler is 2^n
    $60 TIM2_CCMR1 C!
    $60 TIM2_CCMR2 C!
    $60 TIM2_CCMR3 C!
    $11 TIM2_CCER1 C!
    $01 TIM2_CCER2 C!
    1   TIM2_CR1 C!
  ;

  \ Set Timer2 reload value
  : T2Reload ( n -- )
    TIM2_ARRH 2C!
  ;

  \ Set PWM1 compare value
  : PWM1 ( n -- )
    TIM2_CCR1H 2C!
  ;

  \ Set PWM2 compare value
  : PWM2 ( n -- )
    TIM2_CCR2H 2C!
  ;

  \ Set PWM3 compare value
  : PWM3 ( n -- )
    TIM2_CCR3H 2C!
  ;

  \ convert duty cycle [1/1000] to PWM reload value
  : duty ( n -- n )
    TIM2_ARRH 2C@ 1000 */
  ;

ENDTEMP

\\ Example:

 15  T2PwmInit
 1000 T2Reload
 500 pwm2      \ PD3
