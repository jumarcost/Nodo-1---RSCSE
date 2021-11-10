/******************************************************
*CODIGO CORRESPODIENTE AL NODO 1 DE LA ASIGNATURA DE REDES DE SENSORES
*FECHA:   09/11/2021
********************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "sys.h"
#if SAMD || SAMR21 || SAML21
#include "system.h"
#else
#include "led.h"
#include "sysclk.h"
#endif
#include "phy.h"
#include "nwk.h"
#include "sysTimer.h"
#include "sio2host.h"
#include "asf.h"
#include <avr/sleep.h>
#include "temperature.h"

///************Direccion de cada nodo ****************/
#define APP_ADDR                17
#define SINK1_ADDR			     2  

//Variable de control global 
#define ALARM_ON_MSG  'A'
#define ALARM_OFF_MSG 'Z'
#define TEMP_REQ_MSG  'R'
#define ACK_MSG 'K'

uint16_t dst_addr = 0xffff; 
uint8_t x=0;
bool transmision_data=0;


//****** declaracciones prototipo funcione timer alarma ******************///
#define ALARM_TIME 1000        //1s                  
static SYS_Timer_t alarm_timer;
bool alarm_flag = false;
void init_alarm_timer (void);
static void alarm_timer_handler (SYS_Timer_t *timer);
//****** declaracciones prototipo funcione timer alarma ******************///
#define TEMP_SAMPLE_TIME 5000      //5s
static SYS_Timer_t temp_timer;
bool flag_muestreo = false;
void init_temp_timer (void);
static void temp_timer_handler (SYS_Timer_t *timer);
//****** declaracciones prototipo funcione timer alarma ******************///
#define ACK_WAIT_TIME 1000        //1s
static SYS_Timer_t ack_timer;
bool waiting_ack = false;
void init_ack_timer (void);
static void ack_timer_handler (SYS_Timer_t *timer);

/*****************************************************************************/

/********************************************************************************/

/*- Definitions ------------------------------------------------------------*/
#ifdef NWK_ENABLE_SECURITY
  #define APP_BUFFER_SIZE     (NWK_MAX_PAYLOAD_SIZE - NWK_SECURITY_MIC_SIZE)
#else
  #define APP_BUFFER_SIZE     NWK_MAX_PAYLOAD_SIZE
#endif

static uint8_t rx_data[APP_RX_BUF_SIZE];

/*- Types ------------------------------------------------------------------*/
typedef enum AppState_t {
	APP_STATE_INITIAL,
	APP_STATE_IDLE,
} AppState_t;

/*- Prototypes -------------------------------------------------------------*/
static void appSendData(void);

/*- Variables --------------------------------------------------------------*/
static AppState_t appState = APP_STATE_INITIAL;
static SYS_Timer_t appTimer;
static NWK_DataReq_t appDataReq;
static bool appDataReqBusy = false;
static uint8_t appDataReqBuffer[APP_BUFFER_SIZE];
static uint8_t appUartBuffer[APP_BUFFER_SIZE];
static uint8_t appUartBufferPtr = 0;
static uint8_t sio_rx_length;
/*- Implementations --------------------------------------------------------*/

/*************************************************************************//**
*****************************************************************************/
static void appDataConf(NWK_DataReq_t *req)
{
	appDataReqBusy = false;
	(void)req;
}

/*************************************************************************//**
*****************************************************************************/
static void appSendData(void)
{
	if (appDataReqBusy || 0 == appUartBufferPtr) {
		return;
	}

	memcpy(appDataReqBuffer, appUartBuffer, appUartBufferPtr);

	appDataReq.dstAddr = dst_addr;
	
	
	appDataReq.dstEndpoint = APP_ENDPOINT;
	appDataReq.srcEndpoint = APP_ENDPOINT;
	appDataReq.options = NWK_OPT_ENABLE_SECURITY;
	appDataReq.data = appDataReqBuffer;
	appDataReq.size = appUartBufferPtr;
	appDataReq.confirm = appDataConf;
	NWK_DataReq(&appDataReq);

	appUartBufferPtr = 0;
	appDataReqBusy = true;
	
}

/*************************************************************************//**
*****************************************************************************/
static void appTimerHandler(SYS_Timer_t *timer)
{
	appSendData();
	(void)timer;
}

/*************************************************************************//**
*****************************************************************************/
static bool appDataInd(NWK_DataInd_t *ind)
{	
	if (ind->data[0] == ALARM_ON_MSG){
		SYS_TimerStart(&alarm_timer);	
		printf ("\r\n\n\n\n*************************************************** \n\r");
		printf ("\r\n LA ALARMA HA SIDO ACTIVADA POR EL SINK: %d\n\r", ind->srcAddr);
		printf ("\r\n*************************************************** \r");
	}
	if (ind->data[0] == ALARM_OFF_MSG){
		SYS_TimerStop(&alarm_timer);
		alarm_flag = false;
		printf ("\r\n\n\n\n*************************************************** \n\r");
		printf ("\r\n LA ALARMA HA SIDO APAGADA POR EL SINK: %d\n\r", ind->srcAddr);
		printf ("\r\n*************************************************** \r");
		ioport_set_value(LED0, 1);//aPAGAR EL LED
	}
	if (ind->data[0] == TEMP_REQ_MSG){
        pData = read_temperature();
		SYS_TimerStart(&ack_timer);
		transmision_data=1;
		sprintf((char *)appUartBuffer,"X_%02d,%03dC", pData[0], pData[1]);
		appUartBufferPtr = sizeof(appUartBuffer);
		dst_addr = ind->srcAddr;
		appSendData();																							///
		dst_addr = 0xffff;
		printf ("\r\n Respuesta a la peticion de temperatura del Sink %d  \n\r",ind->srcAddr);
	}
		
	if (ind->data[0] == ACK_MSG){
		printf ("\r\n Mensaje entregado al Sink %d  \n\r",ind->srcAddr);
		SYS_TimerStop(&ack_timer);
		x=0;
		transmision_data=0;
		waiting_ack=0;
			
	}	
	return true;
}

/*************************************************************************//**
*****************************************************************************/
static void appInit(void)
{
	NWK_SetAddr(APP_ADDR);
	NWK_SetPanId(APP_PANID);
	PHY_SetChannel(APP_CHANNEL);
#ifdef PHY_AT86RF212
	PHY_SetBand(APP_BAND);
	PHY_SetModulation(APP_MODULATION);
#endif
	PHY_SetRxState(true);

	NWK_OpenEndpoint(APP_ENDPOINT, appDataInd);

	appTimer.interval = APP_FLUSH_TIMER_INTERVAL;
	appTimer.mode = SYS_TIMER_INTERVAL_MODE;
	appTimer.handler = appTimerHandler;
}

/*************************************************************************//**
*****************************************************************************/
static void APP_TaskHandler(void)
{
	switch (appState) {
	case APP_STATE_INITIAL:
	{
		appInit();
		appState = APP_STATE_IDLE;
	}
	break;

	case APP_STATE_IDLE:
		break;

	default:
		break;
	}
	sio_rx_length = sio2host_rx(rx_data, APP_RX_BUF_SIZE);
	if (sio_rx_length) {
		for (uint16_t i = 0; i < sio_rx_length; i++) {
			sio2host_putchar(rx_data[i]);
			if (appUartBufferPtr == sizeof(appUartBuffer)) {
				appSendData();
			}

			if (appUartBufferPtr < sizeof(appUartBuffer)) {
				appUartBuffer[appUartBufferPtr++] = rx_data[i];
			}
		}

		SYS_TimerStop(&appTimer);
		SYS_TimerStart(&appTimer);
	}
}

/*************************************************************************//**
*****************************************************************************/
int main(void)
{
	irq_initialize_vectors();
	#if SAMD || SAMR21 || SAML21
	system_init();
	delay_init();
	#else
	sysclk_init();
	board_init();
	#endif
	SYS_Init();
	sio2host_init();
	cpu_irq_enable();
	
	twi_init();
	sensor_conf();
	init_alarm_timer();
	
	init_temp_timer();
	
	printf ("\r\n *El nodo esta encendido \n\r");
	init_ack_timer();
	
	while (1) {
		SYS_TaskHandler();
		APP_TaskHandler();
		if (alarm_flag){
				LED_Toggle(LED0);
				/*********/
				alarm_flag = false;
			}
				if((transmision_data==1)&&(waiting_ack==1))
				{
					x++;
					waiting_ack=0;
					if(x==3)
					{
						SYS_TimerStop(&ack_timer);
						printf("\r\nFallo de envío de la temperatura al Nodo %d",SINK1_ADDR);
						transmision_data=0;
						x=0;
					}					
				}		
				
		if (flag_muestreo){
			
			pData = read_temperature();  //Sensor 
			
			printf ("\r\n\n\n\n***************************************  Nueva Muestra  *************************************** \n\r");
			printf ("\r\n Temperatura medida en nodo Id=%d [ts=%ds]= %02d,%03d C \n\r",APP_ADDR,TEMP_SAMPLE_TIME/1000, pData[0], pData[1]);
			/**/
			sprintf((char *)appUartBuffer,"T %02d,%03dC", pData[0], pData[1]);
			appUartBufferPtr = sizeof(appUartBuffer);
			if(pData[0]>30){
				printf("La temperatura es mayor que el límite");
				sprintf((char *)appUartBuffer, "A");
				appUartBufferPtr = sizeof(appUartBuffer);
				dst_addr=0xffff;
				appSendData();
				SYS_TimerStart(&alarm_timer);
				printf ("\r\n\n ACTIVO LAS ALARMAS DE TODOS \n\n\r");
			}
			
			dst_addr = SINK1_ADDR;		
			appSendData();																				///
			dst_addr = 0xffff;
			printf ("\r\n Enviado a los nodos Sink %d\n\r",SINK1_ADDR);
			/*******/
			flag_muestreo = false;
			SYS_TimerStart(&ack_timer);
			transmision_data=1;
		}
	}
}

/***************************************************************************************************************/
/***************************************************************************************************************/
/************************************************************************************/
/*********************************************************************************/
/*************************************************************************/
/*****************Funciones auxiliares ***************************/
/**********************************************************/
/****************************************************/
/************************************/		

//ALARMA

void init_alarm_timer (void) {
	
	alarm_timer.interval = ALARM_TIME; // Cuento 1 segundo
	alarm_timer.mode = SYS_TIMER_PERIODIC_MODE; // Modo periódico
	alarm_timer.handler = alarm_timer_handler;
	//SYS_TimerStart(&temporizador_alarma);
	
}

static void alarm_timer_handler (SYS_Timer_t *timer) {
	alarm_flag = true;
}

//MUESTRO


void init_temp_timer (void) {
	
	temp_timer.interval = TEMP_SAMPLE_TIME; 
	temp_timer.mode = SYS_TIMER_PERIODIC_MODE; 
	temp_timer.handler = temp_timer_handler;
	SYS_TimerStart(&temp_timer);
	
}

static void temp_timer_handler (SYS_Timer_t *timer) {
	flag_muestreo = true;
}

void init_ack_timer (void) {
	
	ack_timer.interval = ACK_WAIT_TIME;
	ack_timer.mode = SYS_TIMER_PERIODIC_MODE;
	ack_timer.handler = ack_timer_handler;
	SYS_TimerStart(&ack_timer);	
}

static void ack_timer_handler (SYS_Timer_t *timer) {
	waiting_ack = true;
}
