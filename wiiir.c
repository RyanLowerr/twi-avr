#include <avr/io.h>
#include <util/twi.h>

// TWI genera status codes
#define TWI_START              0x08 // Start has been transmitted.
#define TWI_REP_START          0x10 // Repeated start has been transmitted.
#define TWI_ARB_LOST           0x38 // Arbitration lost.

// TWI master transmitter status codes
#define TWI_MTX_ADR_ACK        0x18 // SLA+W has been transmitted and ACK reveived.
#define TWI_MTX_ARD_NACK       0x20 // SLA+W has been transmitted and NACK reveived.
#define TWI_MTX_DATA_ACK       0x28 // Data byte has been transmitted and ACK reveived.
#define TWI_MTX_DATA_NACK      0x30 // Data byte has been transmitted and NACK reveived.

// TWI master reveiver status codeds
#define TWI_MRX_ADR_ACK        0x40 // SLA+W has been transmitted and ACK reveived.
#define TWI_MRX_ARD_NACK       0x48 // SLA+W has been transmitted and NACK reveived.
#define TWI_MRX_DATA_ACK       0x50 // Data byte has been transmitted and ACK reveived.
#define TWI_MRX_DATA_NACK      0x58 // Data byte has been transmitted and NACK reveived.

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

void twi_start(void)
{
	TWCR = (1 << TWEN) |                               // Enable TWI interface and release TWI pins.
	       (0 << TWIE) | (1 << TWINT) |                // Enable TWI interupt.
	       (0 << TWEA) | (1 << TWSTA) | (0 << TWSTO) | // Enable start condition.
	       (0 << TWWC);
}

ISR(TWI_vect)
{
	switch (TWSR & 0xf8)                               // Chect the TWI status registar while masking off the prescaler bits.
	{
		case TWI_START:
		case TWI_REP_START:
		case TWI_MTX_ADR_ACK:
		case TWI_MTX_DATA_ACK:
		case TWI_MRX_ADR_ACK:
		case TWI_MRX_DATA_ACK:
		case TWI_MTX_ADR_NACK:
		case TWI_MTX_DATA_NACK:
		case TWI_MRX_ADR_NACK:
		case TWI_MRX_DATA_NACK:		
		default:
			twi_init();                                // Reset the TWI.
	}
}

int main(void)
{

	twi_init();

	while(1)
	{
		twi_start();
	}
	
	return 0;
}
