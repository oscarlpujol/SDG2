/*
Cuando se le pide algo el sensor empieza por >

*/

#include "proyecto.h"

static TipoProyecto cait;
static int flags = 0;
/*
static int flag_button_on = 0;
static int flag_button_off = 0;
static int flag_bit = 0;
static int flag_mensaje = 0;
static int flag_time_out = 0;
*/

//static int bits_CO2;
//static int bits_TVOC;

static pthread_mutex_t mutex;
static tmr_t tmr_1;
static int num_bits_recibidos;

// static int bits_CRC[8];

int contador = 0;

enum Estados {
	SLEEP,
	MEASURE,
	ACK_SENDER,
} ;

void delay_until (unsigned int next, unsigned int now) {
	//printf("esperando %d \n",next - now);
	//fflush(stdout);
	if (next > now) {
		usleep((next-now)*1000);
	}
}

int kbhit(void){
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}

// Comprobación de flags

int comprobar_button_on (fsm_t* this){

	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (flags & FLAG_BUTTON_ON);
	pthread_mutex_unlock (&mutex);
	return result;
}

int comprobar_button_off (fsm_t* this){

	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (flags & FLAG_BUTTON_OFF);
	pthread_mutex_unlock (&mutex);
	return result;}

int comprobar_mensaje_recibido (fsm_t* this){

	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (flags & FLAG_MENSAJE);
	pthread_mutex_unlock (&mutex);
	return result;
}

int comprobar_8_bits_recibidos (fsm_t* this){

	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (flags & FLAG_BIT);
	pthread_mutex_unlock (&mutex);
	return result;
}

int comprobar_tiempo (fsm_t* this){

	pthread_mutex_lock (&mutex);
	int result = 0;
	result = (flags & FLAG_TIME_OUT);
	pthread_mutex_unlock (&mutex);
	return result;
}

// Acciones de la máquina de estados

static void init_air_quality (fsm_t* this){

	printf("Enviando 'init_air_quality' al sensor\n");
	fflush(stdout);
	// Preguntar que es eso
	pthread_mutex_lock (&mutex);
	flags &= (~FLAG_BUTTON_ON);
	flags |= (FLAG_TIME_OUT);
	printf("Iniciando... \n");
	fflush(stdout);
	pthread_mutex_unlock (&mutex);
}

static void measure_air_quality (fsm_t* this){

	TipoProyecto *p_cait;
	p_cait = (TipoProyecto*)(this->user_data);
	tmr_startms((tmr_t*)(p_cait->tmr), REFRESH_TIME);

	printf("Enviando petición al sensor\n");
	printf(">Insertar medidas CO2 \n");
	fflush(stdout);

	scanf("%d", &(p_cait->medidaCO2));
	printf("Medida CO2: %d \n", p_cait->medidaCO2);	
	fflush(stdout);

	bits_recibidos();

	pthread_mutex_lock (&mutex);
	flags &= (~FLAG_TIME_OUT);
	pthread_mutex_unlock (&mutex);
/*
	En caso de CRC ponerlo aqui como mas informacion
*/
}

static void power_off (fsm_t* this){

	printf("Apagar el sensor\n");
//	printf("Apagando la máquina\n");
//	exit(0);
	pthread_mutex_lock (&mutex);
	flags &= (~FLAG_BUTTON_OFF);
	printf("Apagando... \n");
	fflush(stdout);
	pthread_mutex_unlock (&mutex);
}

static void mostrar_resultados (fsm_t* this){

	TipoProyecto *p_cait;
	p_cait = (TipoProyecto*)(this->user_data);

	printf("Enviando ACK de TVOC al sensor\n");
	printf("Los resultados obtenidos son:\n CO2: %d ppm\n TVOC: %d ppb\n", (p_cait->medidaCO2),(p_cait->medidaTVOC));

	pthread_mutex_lock (&mutex);
	flags &= (~FLAG_MENSAJE);
	pthread_mutex_unlock (&mutex);
}

static void ack (fsm_t* this){

	TipoProyecto *p_cait;
	p_cait = (TipoProyecto*)(this->user_data);

	pthread_mutex_lock (&mutex);
	flags &= (~FLAG_BIT);
	pthread_mutex_unlock (&mutex);

	printf("Enviando ACK de CO2 al sensor\n"); // Si no se han recibido los 8 bits (debería saberse con bits_recibidos) debería mostrar otro mensaje
	printf(">Insertar medidas TVOC\n");
	fflush(stdout);

	scanf("%d", &(p_cait->medidaTVOC));

	if(flags & FLAG_TIME_OUT){
		printf("TIMEOUT\n");
		num_bits_recibidos = 0;
		pthread_mutex_lock (&mutex);
		flags &= (~FLAG_MENSAJE);
		pthread_mutex_unlock (&mutex);
		return;
	}

	printf("Medida CO2: %d \n", p_cait->medidaTVOC);
	fflush(stdout);
	
	bits_recibidos();

	pthread_mutex_lock (&mutex);
	flags &= (~FLAG_BIT);
	pthread_mutex_unlock (&mutex);	

}

void bits_recibidos (void){
	pthread_mutex_lock (&mutex);
	num_bits_recibidos++;
	pthread_mutex_unlock (&mutex);
	if(num_bits_recibidos < 2){
		pthread_mutex_lock (&mutex);
		flags |= (FLAG_BIT);
		pthread_mutex_unlock (&mutex);
	}
	else{
		mensaje_recibido();
	}
/*
	Debe ser capaz de ver si se han recibido o no los 8 bits correspondientes. Activa flag FLAG_BIT
*/
}

void mensaje_recibido (void){
	pthread_mutex_lock (&mutex);
	if(num_bits_recibidos == 2){
		flags |= (FLAG_MENSAJE);
		num_bits_recibidos = 0;
	}
	pthread_mutex_unlock (&mutex);
/*
	Debe ser capaz de activar el flag de FLAG_MENSAJE
*/
}

// Atención a timer e interrupción

void button (void) {	
	if(kbhit() && getchar() == 32){
		if (cait.fase == ENCENDIDO){
			pthread_mutex_lock (&mutex);
			flags |= (FLAG_BUTTON_OFF);
			cait.fase = APAGADO;
			pthread_mutex_unlock (&mutex);
		}
		else if (cait.fase == APAGADO){
		//	fflush(stdout);
			pthread_mutex_lock (&mutex);
			flags |= (FLAG_BUTTON_ON);
			cait.fase = ENCENDIDO;
			pthread_mutex_unlock (&mutex);
		}
	}	
/*
	Depende del tipo de botón: 
		Tipo boli:
			-Si está apretado encendido, si no, apagado.
		Tipo linterna:
			-Apretar cambia de estado.
*/
}

static void tiempo (union sigval value){

	pthread_mutex_lock (&mutex);
	flags |= (FLAG_TIME_OUT);
	pthread_mutex_unlock (&mutex);
/*
	Si ha sobrepasado el timeout cambia flags
*/
}

int initialize(TipoProyecto *p_cait) {

	p_cait->fase = APAGADO;
	p_cait->tmr = tmr_new (tiempo);
	p_cait->medidaCO2 = 0;
	p_cait->medidaTVOC = 0;
//	tmr_startms((tmr_t*)(p_tmr_1), 5000);
	pthread_mutex_init(&mutex, NULL);


	printf("\nSystem init complete!\n");
	fflush(stdout);

	return 0;
}

int main () {

	fsm_trans_t maquina[] = {
		{ SLEEP, comprobar_button_on, MEASURE, init_air_quality },
		{ MEASURE, comprobar_tiempo, ACK_SENDER, measure_air_quality },
		{ MEASURE, comprobar_button_off, SLEEP, power_off },
		{ ACK_SENDER, comprobar_mensaje_recibido, MEASURE, mostrar_resultados },
		{ ACK_SENDER, comprobar_8_bits_recibidos, ACK_SENDER, ack },
		{ ACK_SENDER, comprobar_tiempo, ACK_SENDER, measure_air_quality },
		{ ACK_SENDER, comprobar_button_off, SLEEP, power_off },
		{-1, NULL, -1, NULL },
	};

	initialize(&cait);

	fsm_t* maquina_fsm = fsm_new (SLEEP, maquina, &cait);

	long a;
	struct timespec spec;
	clock_gettime(CLOCK_REALTIME, &spec);
	a = round(spec.tv_nsec / 1000000);
	unsigned int ms = a;

	while (1) {
		button();
		fsm_fire (maquina_fsm);

		clock_gettime(CLOCK_REALTIME, &spec);
		a = round(spec.tv_nsec / 1000000);
		unsigned int now = ms;

		ms += CLK_MS;
		delay_until (ms,now);
	}
	return 0;
}