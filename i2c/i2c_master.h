/*****************************************************************
 *
 * This software was written by Paul L Daniels - pldaniels@gmail.com
 *
 *
 * Based off the AVR300 Software i2c Master document with a very small
 * change to make it actually work with AVR hardware for when reading
 * more than 1 byte back from the slave.1
 *
 * Supports clock stretching
 *
 * No special hardware required, just two pins with I/O capabilities.
 * 
 * While this code is using Tiny series-1 type pin acccess you should
 * be able to easily change it to any other uC
 *
 * Created for/using avr-gcc tools and avr-libc
 *
 * NOTES
 *
 * Remember to shift addresses to the LEFT by 1 bit so that you can
 * add the R/W bit at bit-0; ie
 *
 *		addr = ( INA_219 << 1 ) | I2C_READ
 *
 * ---------------------------------------------------------------------
 *  RELEASES
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
#include <avr/io.h>

#define I2C_READ 1
#define I2C_WRITE 0

// at 100kHz our clock has a 10us period
// as such our quarter and half delays are
// set accordingly (whole numbers only though)
//
#define Q_DELAY _delay_loop_2(3)
#define H_DELAY _delay_loop_2(5)

// Set your port I/O codes here
//
// This example code is actually 
// quite verbose/ugly/excessive
// but it has been done so in order
// to *really* ensure we do not 
// get quirky behaviours.
//
// You can pare down the code 
// as you want.
//
// **REMEMBER** you're not trying 
// to /actively/ drive the SCL/SDA
// to a high/1 stage, that is done
// by the 4k7~10k pullup resistors
// that should be present on the lines.
//
// To achieve a high state you have 
// to make your port let go of the 
// line so it'll raise up implicitly.
//
// i2c is all about yanking-down a 
// line that by default floats high.
//
//	Most i2c slave devices are incredibly tolerant of 
//	widely different clockspeeds as they're mostly
//	holding static/stable data in the registers
//
//
//
//	I've run things like the INA219 down to about 10kHz
//	and it's fine.  "Results will vary".
//
//
// In this example, we're using the following;
//
//		SCL is on PA2 / PORTA.2
//		SDA is on PA3 / PORTA.3
//
//
uint8_t sda_read(void) {
	return (PORTA.IN & (1<<PIN2_bp));
}
void sda_high(void) {
	PORTA.DIRCLR = (1<<PIN2_bp);
	PORTA.OUTCLR = (1<<PIN2_bp); // redundant to be pedantic
}
void sda_low(void) {
	PORTA.DIRSET = (1 << PIN2_bp);
}

uint8_t scl_read(void) {
	return (PORTA.IN & (1<<PIN3_bp));
}
void scl_high(void) {
	PORTA.DIRCLR = (1<<PIN3_bp);
	PORTA.OUTCLR = (1<<PIN3_bp); // redundant to be pedantic
}
void scl_low(void) {
	PORTA.DIRSET = (1 << PIN3_bp);
}
//
//
//
void i2c_init() {
	sda_high();	
	scl_high();	

	// Set the output configuration
	// of the pins to zero/low so that
	// when the data-direction is set
	// to OUT when we call the sda/scl_low()
	// routines the port is already going
	// to be at zero/low state
	//
	PORTA.OUT &= ~(1<<PIN2_bp);
	PORTA.OUT &= ~(1<<PIN3_bp);
}
//
//
// From here you shouldn't need to 
// change any code, in theory ;)
//
// ---------------------------------------




// i2c_write_byte ( data )
//
// Assumes the nothing about the
// state of the i2c bus but will
// listen out for the ack from a
// slave and return the ack status
//
uint8_t i2c_write_byte(uint8_t data) {

	uint8_t i;

	for(i=0;i<8;i++) {
		scl_low();

		if(data & 0x80) {
			sda_high();
		} else {
			sda_low();	
		}
		H_DELAY;

		scl_high();
		H_DELAY;

		data = data<<1;
	}

	// Data has been sent, now listen for the ACK
	//
	scl_low();
	H_DELAY;

	scl_high();		
	while (scl_read()==0);

	uint8_t ack;
	ack = sda_read();

	H_DELAY;

	return ack;
}



// i2c_stop()
//
// Halts the current i2c transaction
//
//
void i2c_stop() {
	scl_low();
	sda_low();
	H_DELAY;
	scl_high();
	Q_DELAY;
	sda_high();
	H_DELAY;
}



// i2c_read_byte( ack )
//
// Set ack = 1 if you want to read more bytes after this read
// Set ack = 0 if you're done with this i2c transaction
//
// Return value is the read byte
//
uint8_t i2c_read_byte(uint8_t ack) {
	uint8_t data=0x00;
	uint8_t i;

	for(i=0;i<8;i++) {

		scl_low();
		sda_high(); // Without this, the uC sent ACK doesn't seem to work
		H_DELAY;
		scl_high();
		H_DELAY;

		if (sda_read()) data |= 1;
		if (i < 7) data <<= 1;
	}

	// Send the ack
	//
	scl_low();
	if (ack == 1) sda_low();
	else sda_high();
	H_DELAY;

	scl_high();
	while (scl_read() == 0); // wait for slave to finish if it's stretching
	H_DELAY;

	return data;
}



// i2c_start( addr ) 
//
// Start a new i2c transaction, hopefully 
// with the device having the address addr.
//
// NOTE: you need to shift your device addr
// by 1 bit to the left and add/OR the R/W
// bit to the end
//
// ie,  device has addr of 0x40, so we have 
// to  make addr using;
//			i2c_start( (0x40<<1)|I2C_WRITE )
//
//
uint8_t i2c_start( uint8_t addr ) {
	sda_low();
	Q_DELAY;
	return i2c_write_byte(addr);
}



// i2c_start_rep( addr )
//
// Invoke a repeat start
//
uint8_t i2c_start_rep( uint8_t addr ) {
	scl_low();
	sda_high();
	H_DELAY;
	scl_high();
	Q_DELAY;
	sda_low();
	Q_DELAY;

	return i2c_write_byte(addr);
}



// i2c_start_wait( addr )
//
// Start a new i2c transaction but wait for
// the slave device to respond
//
// WARNING: if the slave doesn't respond this
// will endlessly loop :(
//
// Perhaps in future we'll add a timeout count.
//
uint8_t i2c_start_wait( uint8_t addr ) {
	while ( 1 ) {
		sda_low();
		H_DELAY;
		if (i2c_write_byte(addr) == 0) break;
		i2c_stop();
	}
	return 0;
}

//-----------------END of i2c_master.h----------------
