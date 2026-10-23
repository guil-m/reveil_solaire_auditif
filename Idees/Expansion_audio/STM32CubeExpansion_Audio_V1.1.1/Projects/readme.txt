The STM32F7xx devices can reach a maximum clock frequency of 216MHz.
Exception is made for the examples/ Applications that are dealing with SDRAM or using it.
Indeed proper functioning of the SDRAM is only guaranteed at a maximum system clock frequency of 200MHz.

Given that the LCD mounted on the STM32F7xxx-EVAL and STM32F7xxx-Discovery boards require the use of SDRAM,
audio examples and applications are bound by its maximum reachable clock frequency (200MHz).

The STM32F4xx devices can reach a maximum clock frequency of 180MHz.