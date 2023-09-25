# STM8 eForth experiments with the XY-LPWM board

[![Travis-CI](https://travis-ci.org/TG9541/XY-LPWM.svg?branch=master)](https://travis-ci.org/TG9541/XY-LPWM)

![XY-LPWM](https://raw.githubusercontent.com/wiki/plumbum/stm8ef/helo_forth.jpg)

## Hardware

J3 ICP Pin|Signal
-|-
1|+3.3V (supply)
2|NRST
3|SWIM and [serial console](https://github.com/TG9541/stm8ef/wiki/STM8S-Value-Line-Gadgets#other-target-boards)
4|STM8S003F3P6 Vcap (do not connect)
5|GND

Recently XY-LPWM modules are based on the Nuvoton N76E003AT20 controller which is a MCS51 type "pin-compatible" replacement for the STM8S003F3P6 which is otherwise incompatible. A [new board](https://github.com/TG9541/XY-LPWM/issues/1) has appeared that uses an unknown µC in an SO16 package. This means it's getting more risky to use this board if you intend modify it.

![schematics](https://protosupplies.com/wp-content/uploads/2019/09/XY-LPWM-Schematic.jpg)

This chip must be replaced for using STM8 eForth.

## Building

Run `make` on a Linux system to...

* pull dependencies
* build the STM8 eForth board package
* transfer board Forth code from `XY-LPWM/board.fs` 


## STM8 eForth Console

In order to free up the STM8S UART for development, the Forth console uses a simulated half-duplex serial interface on PD1/SWIM. The serial console and an ST-LINK V2 (or a corresponding USB dongle) can be connected in parallel:

```
XY-LPWM        .      .----o serial TxD "TTL"
               .      |      (e.g. "PL2303" USB serial converter)
               .     ---
               .     / \  e.g. 1N4148
               .     ---
ICP header     .      |
               .      *----o serial RxD "TTL
               .      |
VCC------------>>-----+----o ST-LINK 3.3V
               .      |
Vcap----------->> NC  |
               .      |
STM8 PD1/SWIM-->>-----*----o ST-LINK SWIM
               .
NRST----------->>----------o ST-LINK NRST
               .
GND------------>>-----*----o ST-LINK GND
               .      |
................      .----o serial GND
```
