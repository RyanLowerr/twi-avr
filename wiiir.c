#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

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

#define TWI_ACK  1
#define TWI_NACK 0

static volatile uint8_t twi_tx_buffer[128];
static volatile uint8_t twi_rx_buffer[128];
static volatile uint8_t twi_tx_pointer;
static volatile uint8_t twi_rx_pointer;
static volatile uint8_t twi_tx_length;

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

static void twi_start(void)
{
	TWCR = (1 << TWEN) |                               // Enable TWI interface and release TWI pins.
	       (1 << TWIE) | (1 << TWINT) |                // Enable TWI interupt and clear the interupt flag.
	       (0 << TWEA) | (1 << TWSTA) | (0 << TWSTO) | // Enable start condition.
	       (0 << TWWC);
}

static void twi_stop(void)
{
	TWCR = (1 << TWEN) |                                  // Enable TWI interface and release TWI pins.
	       (0 << TWIE) | (1 << TWINT) |                   // Disable TWI interupt and clear the interupt flag.
	       (0 << TWEA) | (0 << TWSTA) | (1 << TWSTO) |    // Send stop condition.
	       (0 << TWWC);
}

static void twi_enable_interupt(uint8_t ack)
{
	if(ack)
	{
		TWCR = (1 << TWEN) |                               // Enable TWI interface and release TWI pins.
			   (1 << TWIE) | (1 << TWINT) |                // Enable TWI interupt and clear the interupt flag.
			   (1 << TWEA) | (0 << TWSTA) | (0 << TWSTO) |
			   (0 << TWWC);
	}
	else
	{
		TWCR = (1 << TWEN) |                               // Enable TWI interface and release TWI pins.
			   (1 << TWIE) | (1 << TWINT) |                // Enable TWI interupt and clear the interupt flag.
			   (0 << TWEA) | (0 << TWSTA) | (0 << TWSTO) |
			   (0 << TWWC);
	}
}

void twi_write(uint8_t sla, uint8_t* data, uint8_t length)
{
	// Copy slave address into the tx buffer.
	twi_tx_buffer[0] = sla << 1;
	
	// Copy data to be transmitted into the tx buffer.
	for(uint8_t i = 1; i <= length; i++)
		twi_tx_buffer[i] = data[i-1];		
	
	// Save the data length for use in the interupt routine.
	twi_tx_length = length + 1;
	
	// Initiate transmission by issusing a start condition.
	twi_start();
	
	// Should have a status return here...
}

void twi_read(uint8_t sla, uint8_t* data, uint8_t* length)
{
	// Copy slave address into the tx buffer.
	twi_tx_buffer[0] = (sla << 1) + 1;
	
	// Initiate transmission by issusing a start condition.
	twi_start();
	
	// Should wait for reply here... Have a timeout...
	
	// Copy received bytes from buffer to *data
	*length = twi_rx_pointer;
	for(uint8_t i = 0; i < twi_rx_pointer; i++)
		data[i] = twi_rx_buffer[i];
		
	// Should have a status return here...
}

ISR(TWI_vect)
{
	// Chect the TWI status registar while masking off the prescaler bits.
	switch (TWSR & 0xF8)
	{
		// Start condition transmitted.
		case TWI_START:
		// Repeated start condition transmitted.
		case TWI_REP_START:
			// Reset the buffer pointer.
			twi_tx_pointer = 0;
		
		// Slave address and write transmitted. Ack received.
		case TWI_MTX_ADR_ACK:
		// Data byte transmitted. Ack received.
		case TWI_MTX_DATA_ACK:
			// Transmit byte in the buffer or issue twi stop if last data byte has been transmitted.
			if(twi_tx_pointer < twi_tx_length)
			{
				TWDR = twi_tx_buffer[twi_tx_pointer++];
				twi_enable_interupt(TWI_NACK);
			}
			else
				twi_stop();
			break;
		
		// Data byte received. Ack transmitted
		case TWI_MRX_DATA_ACK:
			// Read the last data byte received.
			twi_tx_buffer[twi_tx_pointer++] = TWDR; 
		
		// Slave address and read received. Ack received.
		case TWI_MRX_ADR_ACK:
			// After last data byte received transmitic nack.
			if(twi_tx_pointer < (twi_tx_length - 1))
				twi_enable_interupt(TWI_ACK);
			else
				twi_enable_interupt(TWI_NACK);
			break;			
		
		// Data byte received. Nack transmitted
		case TWI_MRX_DATA_NACK:
			twi_tx_buffer[twi_tx_pointer] = TWDR;
			twi_stop();
			break;
		
		// Slave address and write transmitted. Nack received.
		case TWI_MTX_ADR_NACK:
		// Slave address and read transmitted. Nack received. 
		case TWI_MRX_ADR_NACK:
		// Data byte transmitted. Nack received.
		case TWI_MTX_DATA_NACK:		
		default:
			// Reset the TWI.
			twi_init();
	}
}

int main(void)
{
	twi_init();  // Initalize the TWI.
	sei();       // Enable interrupts.

	uint8_t sla = 112; // slave address
	uint8_t tx[128];
	uint8_t rx[128];
	uint8_t rx_len;

	while(1)
	{
		tx[0] = 1;
		twi_write(sla, &tx[0], 1);
		_delay_ms(.2);
 
		twi_read(sla, &rx[0], &rx_len);
		_delay_ms(.2);
	}
	
	return 0;
}
