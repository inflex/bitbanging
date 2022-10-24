# bitbanging
A set of C routines for providing useful facilities via bit-banging instead of specific hardware ( i2c master, uart tx ).
While the code tries to be as generic as reasonable, it has been written around the avr-gcc / libc toolchain and Atmel Tiny AVR 1-series chips.  To avoid the code becoming a visually complex mess and thus unreadable for new people I've deliberately avoided having all manner of self-configuring/detecting routines to determine the specific ports/functions to use.  Most people should be able to read the code and adapt if required for their specific situation.

## uart.h
Very simple transmit-only, no checks, 9600-8n1 function.  Can be useful when you're in a fix and want to get data out via a serial port but either you've routed the pins elsewhere, don't have hardware support or you just don't care to use the integrated hardware.

Set the SET_0, SET_1, and DELAY defines to suit your setup, likewise the F_CPU/F_OSC.

Created with the AVR Tiny414 / 1-Series AVRs

## i2c_master.h
Should work on any pair of uC pins that support input and output.  This was based off the flow-chart/sequences provided by the AVR300 document and one small adjustment in the byte-read sequence that made the multi-byte read sequences actually work ( before they wouldn't continue even when you had 'ack' set ).

Created with the AVR Tiny414 / 1-Series AVRs
