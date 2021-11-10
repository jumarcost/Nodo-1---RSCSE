/*
 * temperature.h
 *
 * Created: 29/10/2021 13:12:30
 * Author: Javier L�zaro Fern�ndez & Sa�l Vikran L�pez Hidalgo
 * 
 */ 
#include <stdlib.h>
#include <stdint.h>
#include <twi_megarf.h>
#include <stdlib.h>
//#include <util/delay.h>
#include <stdint.h>
#include <sysclk.h>

/************************************************************************/
/* Definiciones para el Two Wire Interface								*/
/************************************************************************/
#define TWI_MASTER				&TWBR
#define TWI_SPEED				125000
#define TWI_SLAVE_ADDR			0x96
#define SLAVE_MEM_ADDR			0x01
#define SLAVE_MEM_ADDR_LENGTH   TWI_SLAVE_ONE_BYTE_SIZE
#define DATA_LENGTH  sizeof(conf_data)

/************************************************************************/
/* Funciones                                                            */
/************************************************************************/
extern void sensor_conf (void); // Configuraci�n del sensor de temperatura
extern uint8_t* read_temperature (void); // Leer el dato de la temperatura
extern uint16_t adc_meastemp (void); // Leer temperatura procesador
extern void twi_init (void); // Inicializar el Two Wire Interface

//Variables que se usaran en el proyecto 
 uint8_t* pData;
 uint16_t temperatura_procesador;
 float grados_Celsius;
 uint16_t parteEntera;
 uint16_t parteDecimal;
 uint8_t msg[256];
 uint8_t msg2[256];
/*
Añadir al main.c



 // Para la conversi�n de la temperatura anterior a �C

// Parte entera y decimal de la temperatura del uC






////////////////////////////////////////////////////////////////////////////////

Añadir al main antes del bucle

twi_init();
sensor_conf();

		pData = read_temperature(); // Leer temperatura sensor
		temperatura_procesador = adc_meastemp(); // Leer temperatura procesador
		grados_Celsius = (1.13 * temperatura_procesador) - 272.8; // Conversi�n a C
		parteEntera = (uint16_t)grados_Celsius; // Extraer parte entera
		parteDecimal = (uint16_t)((grados_Celsius - parteEntera)*1000); // Extraer parte decimal
	
		sprintf(msg,"%c%c\n%d%s%02d,%03d\n\r%c%c", 0x10, 0x02, 12, "T_", pData[0], pData[1], 0x03, 0x10);
		sprintf(msg2,"%c%c\n%d%s%02d,%03d\n\r%c%c", 0x10, 0x02, 12, "T_", parteEntera, parteDecimal, 0x03, 0x10);
		
#if DEBUG

		printf("[INFO] main | Sending msg [Sensor Temp]: %s\n", msg);
		printf("[INFO] main | Sending msg2 [uC Temp]: %s\n", msg2);

#endif
		strcpy(message,msg);
		sendFrame();
		strcpy(message,msg2);
		sendFrame();


*/
