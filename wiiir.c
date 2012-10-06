#include <avr/io.h>
#include <util/twi.h>

void twi_init(void)
{
	// SCL freq = (CPU Clock) / (16 + 2(TWBR) * (prescale value))
	// 400 kHz = (16MHz) / (16 + 2(12) * (1))
	TWBR = 12;
	TWSR = 0;   // TWPS = 0 => prescale value = 1 
	TWCR = (1 << TWEN) |                               // Enable TWI interface and release TWI pins.
	       (0 << TWIE) | (0 << TWINT) |                // Disable TWI interupt.
	       (0 << TWEA) | (0 << TWSTA) | (0 << TWSTO) | // No signal requests.
	       (0 << TWWC);
}

uint8_t twi_busy(void)
{
	return (TWCR & (1 << TWIE));                       // TWI is busy when TWIE is enabled.
}

void twi_start(void)
{
	while(twi_busy());                                 // Wait untill TWI is ready.
	
	TWCR = (1 << TWEN) |                               // Enable TWI interface and release TWI pins.
	       (1 << TWIE) | (1 << TWINT) |                // Enable TWI interupt and clear interupt flag.
	       (0 << TWEA) | (1 << TWSTA) | (0 << TWSTO) | // Enable start condition.
	       (0 << TWWC);
}

int main(void)
{

	twi_init();
	twi_start();
		
	while(1)
	{
		;
	}
	
	return 0;
}
