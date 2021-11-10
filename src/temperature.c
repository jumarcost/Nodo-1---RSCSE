/*
 * temperature.c
 *
 * Created: 29/10/2021 13:12:30
 * Author: Javier Lázaro Fernández & Saúl Vikran López Hidalgo
 */

#include "temperature.h"


/************************************************************************/
/*                                                                      */
/************************************************************************/

const uint8_t conf_data[] = {
	0x60
};

/************************************************************************/
/*                                                                      */
/************************************************************************/

typedef struct temp_ctx_t{
	uint8_t readData[2];
} TEMP_CTX_T;

/************************************************************************/
/*                                                                      */
/************************************************************************/


TEMP_CTX_T temp_ctx;

/************************************************************************/
/*                                                                      */
/************************************************************************/

void sensor_conf (void){
	/* configures the TWI configuration packet*/
	twi_package_t packet = {
		.addr[0] = (uint8_t) SLAVE_MEM_ADDR,
		.addr_length = (uint8_t)SLAVE_MEM_ADDR_LENGTH,
		.chip = TWI_SLAVE_ADDR,
		.buffer = (void *)conf_data,
		.length = DATA_LENGTH
	};
	/* Perform a multi-byte write access */
	while (twi_master_write(TWI_MASTER,&packet) != TWI_SUCCESS) {
	}
	/* waits for write completion*/
	//_delay_ms(5);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

uint8_t* read_temperature (void){
	uint8_t received_data[2] = {0, 0};
	
	
	/* configures the TWI read packet*/
	twi_package_t packet_received = {
		.addr[0] = 0x00,
		.addr_length = (uint8_t)SLAVE_MEM_ADDR_LENGTH,
		.chip = TWI_SLAVE_ADDR,
		.buffer = received_data,
		.length = 2,
	};
	/* Perform a multi-byte read access*/
	while (twi_master_read(TWI_MASTER,&packet_received) != TWI_SUCCESS) {
	}
	temp_ctx.readData[0] = received_data[0];
	temp_ctx.readData[1] = received_data[1];
	
	// SE PUEDE IMPRIMIR POR EL HIPERTERMINAL AQU?, PERO LO HAREMOS EN EL MAIN
	return temp_ctx.readData;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

void twi_init (void){
	/* TWI master initialization options. */
	twi_master_options_t m_options = {
		.speed      = TWI_SPEED,
		.chip  = TWI_SLAVE_ADDR,
	};
	m_options.baud_reg = TWI_CLOCK_RATE(sysclk_get_cpu_hz(), m_options.speed);
	/* Enable the peripheral clock for TWI module */
	sysclk_enable_peripheral_clock(TWI_MASTER);
	/* Initialize the TWI master driver. */
	twi_master_init(TWI_MASTER,&m_options);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

uint16_t adc_meastemp (void) {
	ADCSRC = 10<<ADSUT0; // set start-up time
	ADCSRB = 1<<MUX5; // set MUX5 first
	ADMUX = (3<<REFS0) + (9<<MUX0); // store new ADMUX, 1.6V AREF
	// switch ADC on, set prescaler, start conversion
	ADCSRA = (1<<ADEN) + (1<<ADSC) + (4<<ADPS0);
	do {} while( (ADCSRA & (1<<ADSC))); // wait for conversion end
	ADCSRA = 0; // disable the ADC
	return (ADC);
}
