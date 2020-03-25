#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
// #include <wiringPi.h>

#include "fsm.h"
#include "tmr.h"

#define CLK_MS 5;

static int flags = 0;
static int flag_button_on = 0;
static int flag_button_off = 0;
static int flag_bit = 0;
static int flag_mensaje = 0;
static int flag_time_out = 0;

int contador = 0;

enum Estados {
	SLEEP,
	MEASURE,
	ACK_SENDER,
} ;

void delay_until (unsigned int next) {
	long ms1;
	struct timespec spec1;
	clock_gettime(CLOCK_REALTIME, &spec1);
	ms1 = round(spec1.tv_nsec / 1000000);
	unsigned int now = ms1;
	while (next > now) {
		clock_gettime(CLOCK_REALTIME, &spec1);
		now = round(spec1.tv_nsec / 1000000);
	}
}

int comprobar_button_on (fsm_t* this){
	return flag_button_on;
}

int comprobar_button_off (fsm_t* this){
	return flag_button_off;
}

int comprobar_mensaje_recibido (fsm_t* this){
	return flag_mensaje
}

int comprobar_8_bits_recibidos (fsm_t* this){
	return flag_bit;
}

int comprobar_tiempo (fsm_t* this){
	return flag_time_out;
}

static void button (void) {	
/*
	Depende del tipo de botón: 
		Tipo boli:
			-Si está apretado encendido, si no, apagado.
		Tipo linterna:
			-Apretar cambia de estado.
*/
}

static void tiempo (void){
/*
	Si ha sobrepasado el timeout return 1, mientras, return 0
*/
}

static void bits_recibidos (void){
/*
	Recibe un bit y lo suma a un contador, cuando llega a 8 devuelve un 1 y suma 1 al contador de paquetes.
*/
}

static void mensaje_recibido (void){
/*
	Cuando el contador de paquetes es el número de paquetes que necesitamos enviar, return 1;
*/
}


int main () {

	fsm_trans_t maquina[] = {
		{ SLEEP, comprobar_button_on, MEASURE, init_air_quality },
		{ MEASURE, comprobar_tiempo, SLEEP, measure_air_quality },
		{ MEASURE, comprobar_button_off, ACK_SENDER, power_off },
		{ ACK_SENDER, comprobar_mensaje_recibido, MEASURE, mostrar_resultados },
		{ ACK_SENDER, comprobar_8_bits_recibidos, ACK_SENDER, ack },
		{ ACK_SENDER, comprobar_tiempo, ACK_SENDER, measure_air_quality },
		{ ACK_SENDER, comprobar_button_off, SLEEP, power_off },
		{-1, NULL, -1, NULL },
	};

	initialize(&teclado);

	fsm_t* maquina_fsm = fsm_new (SLEEP, maquina, &teclado);


	unsigned int ms;
	struct timespec spec;
	clock_gettime(CLOCK_REALTIME, &spec);
	ms = round(spec.tv_nsec / 1000000);

	while (1) {
		fsm_fire (maquina_fsm);

		ms += CLK_MS;
		delay_until (ms);
	}
	return 0;
}