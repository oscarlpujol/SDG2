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

static int bits_CO2[8];
static int bits_TVOC[8];

static pthread_mutex_t mutex;
static tmr_t tmr_1;

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

static void init_air_quality (fsm_t* this){

	printf("Enviando 'init_air_quality' al sensor\n");
	fflush(stdout);
	// Preguntar que es eso
	pthread_mutex_lock (&mutex);
	flags &= (~FLAG_BUTTON_ON);
	printf("Iniciando... \n");
	fflush(stdout);
	pthread_mutex_unlock (&mutex);
}

static void measure_air_quality (fsm_t* this){
	
	tmr_startms((tmr_t*)(p_cait->tmr), REFRESH_TIME);
	printf("Enviando petición al sensor\n");
	int c;
	printf(">Insertar medidas CO2\n");
	scanf("%8s", bits_CO2);
	printf("Medida CO2: %s \n", bits_CO2);	
	fflush(stdout);
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

	printf("Enviando ACK de TVOC al sensor\n");
	printf("Los resultados obtenidos son:\n CO2: %d \n TVOC: %d \n", /*CO2 y TVOC en bonito*/);
	pthread_mutex_lock (&mutex);
	flags &= (~FLAG_MENSAJE);
	pthread_mutex_unlock (&mutex);
}

static void ack (fsm_t* this){
	printf("Enviando ACK de CO2 al sensor\n");
	int c;
	printf(">Insertar medidas TVOC\n");
	scanf("%8s", bits_TVOC);
	printf("Medida TVOC: %s \n", bits_TVOC);	
	fflush(stdout);
	pthread_mutex_lock (&mutex);
	flags &= (~FLAG_BIT);
	pthread_mutex_unlock (&mutex);	
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

int initialize(TipoProyecto* p_cait) {

	p_cait -> fase = APAGADO;
	p_cait -> tmr = tmr_new (timer_isr);
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
		fsm_fire (maquina_fsm);

		clock_gettime(CLOCK_REALTIME, &spec);
		a = round(spec.tv_nsec / 1000000);
		unsigned int now = ms;

		ms += CLK_MS;
		delay_until (ms,now);
	}
	return 0;
}