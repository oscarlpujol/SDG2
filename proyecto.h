/*

 */

#ifndef _PROYECTO_H
#define _PROYECTO_H

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
// #include <wiringPi.h>

#include "fsm.h"
#include "tmr.h"

#define CLK_MS 5;
#define FLAG_BUTTON_ON = 0x01;
#define FLAG_BUTTON_OFF = 0x02;
#define FLAG_BIT = 0x03;
#define FLAG_MENSAJE = 0x04;
#define FLAG_TIME_OUT = 0x05;
#define REFRESH_TIME = 30000;

enum Boton {
	APAGADO,
	ENCENDIDO,
}

typedef struct 
{int fase;
tmr_t* tmr;		
}TipoProyecto;



#endif /* _TECLADO_TL04_H_ */