#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "twi.h"

int main(void)
{
	twi_init();  // Initalize the TWI.
	sei();       // Enable interrupts.

	uint8_t sla = 0x58; // slave address
	uint8_t tx[128];
	uint8_t rx[128];

	tx[0] = 0x30;
	tx[1] = 0x01;
	twi_write(sla, &tx[0], 2);
	_delay_ms(100);
		
	tx[0] = 0x00;
	tx[1] = 0x00;
	tx[2] = 0x00;
	tx[3] = 0x00;
	tx[4] = 0x00;
	tx[5] = 0x00;
	tx[6] = 0x00;
	tx[7] = 0x90;
	twi_write(sla, &tx[0], 8);
	_delay_ms(100);
		
	tx[0] = 0x07;
	tx[1] = 0x00;
	tx[2] = 0x41;
	twi_write(sla, &tx[0], 3);
	_delay_ms(100);
		
	tx[0] = 0x1A;
	tx[1] = 0x40;
	tx[2] = 0x00;
	twi_write(sla, &tx[0], 3);
	_delay_ms(100);
		
	tx[0] = 0x33;
	tx[1] = 0x03;
	twi_write(sla, &tx[0], 2);
	_delay_ms(100);
	
	tx[0] = 0x30;
	tx[1] = 0x08;
	twi_write(sla, &tx[0], 2);
	_delay_ms(100);
	
	while(1)
	{
	
		tx[0] = 0x37;
		twi_write(sla, &tx[0], 1);
		_delay_us(25);
	
		twi_read(sla, &rx[0], 8);
		_delay_us(380);
		
		twi_read(sla, &rx[0], 4);
		_delay_us(380);
	}
	
	return 0;
}
