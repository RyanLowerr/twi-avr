#include <avr/io.h>
#include <avr/interrupt.h>

// TWI genera status codes
#define TWI_START              0x08 // Start has been transmitted.
#define TWI_REP_START          0x10 // Repeated start has been transmitted.
#define TWI_ARB_LOST           0x38 // Arbitration lost.

// TWI master transmitter status codes
#define TWI_MTX_ADR_ACK        0x18 // SLA+W has been transmitted and ACK reveived.
#define TWI_MTX_ADR_NACK       0x20 // SLA+W has been transmitted and NACK reveived.
#define TWI_MTX_DATA_ACK       0x28 // Data byte has been transmitted and ACK reveived.
#define TWI_MTX_DATA_NACK      0x30 // Data byte has been transmitted and NACK reveived.

// TWI master reveiver status codeds
#define TWI_MRX_ADR_ACK        0x40 // SLA+W has been transmitted and ACK reveived.
#define TWI_MRX_ADR_NACK       0x48 // SLA+W has been transmitted and NACK reveived.
#define TWI_MRX_DATA_ACK       0x50 // Data byte has been transmitted and ACK reveived.
#define TWI_MRX_DATA_NACK      0x58 // Data byte has been transmitted and NACK reveived.

static volatile uint8_t twi_transceiver_buffer[128];
static volatile uint8_t twi_transceiver_pointer;
static volatile uint8_t twi_transceiver_length;

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
	       (1 << TWIE) | (1 << TWINT) |                // Enable TWI interupt and clear the interupt flag.
	       (0 << TWEA) | (1 << TWSTA) | (0 << TWSTO) | // Enable start condition.
	       (0 << TWWC);
}

void twi_stop(void)
{
	TWCR = (1 << TWEN) |                               // Enable TWI interface and release TWI pins.
	       (0 << TWIE) | (1 << TWINT) |                // Disable TWI interupt and clear the interupt flag.
	       (0 << TWEA) | (0 << TWSTA) | (1 << TWSTO) | // Enable stop condition.
	       (0 << TWWC);
}

void twi_write(uint8_t* data, uint8_t length)
{
	for(uint8_t i = 0; i < length; i++)                // Copy data to be transmitted into the transceiver buffer.
		twi_transceiver_buffer[i] = data[i];		
	twi_transceiver_length = length;                    // Save the data length for use in the interupt routine.
	twi_start();                                       // Initiate transmission by issusing a start condition.
}

void twi_read(uint8_t* data, uint8_t length)
{
	;
}

ISR(TWI_vect)
{
	switch (TWSR & 0xF8)                               // Chect the TWI status registar while masking off the prescaler bits.
	{
		case TWI_START:
		case TWI_REP_START:
			twi_transceiver_pointer = 0;
			break;
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

	twi_init();  // Initalize the TWI.
	sei();       // Enable interrupts.

	while(1)
	{
		;
	}
	
	return 0;
}
