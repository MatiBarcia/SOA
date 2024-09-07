#include <Wire.h>
#include <MPU6050.h>
#include <pitches.h>

#define PIN_BOTON 2
#define PIN_BUZZER 11
#define PIN_LED 13

#define TMP_DELAY_BOTON_MILI 50
#define TMP_DELAY_EVENTOS 50

//-------------------------------------------------
// BORRAR ESTO ANTES DE ENTREGAR
//-------------------------------------------------
void verificar_estado_sensor_boton();
void verificar_estado_sensor_acelerometro();

//-------------------------------------------------
// Enums
//-------------------------------------------------

enum estados_e
{
    ESTADO_EMBEBIDO_REPOSO,
    ESTADO_EMBEBIDO_ALERTA_SONANDO,
    ESTADO_EMBEBIDO_ALERTA_FINALIZADA
};

enum eventos_e
{
    EVENTO_BOTON,
    EVENTO_CAIDA,
    EVENTO_TIMEOUT_ALARMA,
    EVENTO_VACIO
};

//-------------------------------------------------
// Estructura de sensores
//-------------------------------------------------

typedef struct sensor_boton_t
{
    int pin;
    int actual;
    int anterior;
} sensor_boton_t;

typedef struct sensor_mpu6050_t
{
    MPU6050 mpu;
} sensor_mpu6050_t;

//-------------------------------------------------
// Estructura de actuadores
//-------------------------------------------------

typedef struct actuador_buzzer_t
{
    int pin;
} actuador_buzzer_t;

typedef struct actuador_led_t
{
    int pin;
    int estado;
} actuador_led_t;

//-------------------------------------------------
// Variables globales
//-------------------------------------------------

sensor_boton_t sensor_boton;
sensor_mpu6050_t sensor_mpu6050;
actuador_buzzer_t actuador_buzzer;
actuador_led_t actuador_led;

unsigned long tiempo_anterior;
unsigned long tiempo_actual;
unsigned long tiempo_delay_boton;

int nota_actual = 0;
unsigned long tiempo_inicio_nota;
bool alerta_sonando = false;

estados_e estado;
eventos_e evento;

//-------------------------------------------------
// Funcion inicial
//-------------------------------------------------

void start()
{
    Serial.begin(9600);

    // Setup sensores
    sensor_boton.pin = PIN_BOTON;
    pinMode(sensor_boton.pin, INPUT_PULLUP);
    sensor_boton.anterior = LOW;

    sensor_mpu6050.mpu.initialize(ACCEL_FS::A16G, GYRO_FS::G2000DPS);

    // Setup actuadores
    actuador_buzzer.pin = PIN_BUZZER;
    pinMode(actuador_buzzer.pin, OUTPUT);

    actuador_led.pin = PIN_LED;
    pinMode(actuador_led.pin, OUTPUT);
    actuador_led.estado = LOW;

    // Se inicializa el temporizador
    tiempo_anterior = millis();
    tiempo_inicio_nota = millis();
}

void sonar_alerta()
{
    unsigned long ahora = millis(); // Obtiene el tiempo actual

    if (nota_actual < sizeof(duraciones) / sizeof(int))
    {
        int duracion = 1000 / duraciones[nota_actual]; // Calcula la duración de la nota

        if (!alerta_sonando)
        {
            tone(PIN_BUZZER, melodia[nota_actual], duracion);    // Inicia la nueva nota con duración
            tiempo_inicio_nota = ahora;                             // Almacena el tiempo de inicio de la nota
            alerta_sonando = true;                               // Indica que una nota está en reproducción
        }
        else
        {
            if (ahora - tiempo_inicio_nota >= duracion)
            {                               // Si ha pasado el tiempo de duración de la nota
                noTone(PIN_BUZZER);         // Detén el tono actual
                nota_actual++;              // Avanza a la siguiente nota
                alerta_sonando = false;     // Indica que la nota terminó
            }
        }
    }
    else
    {
        nota_actual = 0;
    }
}

//-------------------------------------------------
// Funcion de la maquina de estados
//-------------------------------------------------
void fsm()
{
    tomar_eventos();

    switch (estado)
    {
    case ESTADO_EMBEBIDO_REPOSO:
        // SI EVENTO_BOTON O EVENTO_CAIDA, CAMBIA DE ESTADO A ESTADO_EMBEBIDO_ALERTA_SONANDO
        switch (evento)
        {
        case EVENTO_BOTON:
            estado = ESTADO_EMBEBIDO_ALERTA_SONANDO;
            break;
        case EVENTO_CAIDA:
            estado = ESTADO_EMBEBIDO_ALERTA_SONANDO;
            break;
        case EVENTO_TIMEOUT_ALARMA:
            break;
        case EVENTO_VACIO:
            break;
        default:
            break;
        }
        break;
    case ESTADO_EMBEBIDO_ALERTA_SONANDO:
        // SI EVENTO_BOTON, CAMBIA DE ESTADO A EMBEBIDO_ALERTA_FINALIZADA
        switch (evento)
        {
        case EVENTO_BOTON:
            estado = ESTADO_EMBEBIDO_ALERTA_FINALIZADA;
            break;
        case EVENTO_CAIDA:
            break;
        case EVENTO_TIMEOUT_ALARMA:
            estado = ESTADO_EMBEBIDO_ALERTA_FINALIZADA;
            break;
        case EVENTO_VACIO:
            sonar_alerta();
            break;
        default:
            break;
        }
        break;
    case ESTADO_EMBEBIDO_ALERTA_FINALIZADA:
        // FRENAR ALERTA
        switch (evento)
        {
        case EVENTO_BOTON:
            break;
        case EVENTO_CAIDA:
            break;
        case EVENTO_TIMEOUT_ALARMA:
            break;
        case EVENTO_VACIO:
            estado = ESTADO_EMBEBIDO_REPOSO;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

//-------------------------------------------------
// Funciones definidas por nosotros
//-------------------------------------------------

int indice = 0;
void (*verificar_sensor[2])() = {verificar_estado_sensor_boton, verificar_estado_sensor_acelerometro};

void tomar_eventos()
{
    tiempo_actual = millis();

    if (tiempo_actual - tiempo_anterior > TMP_DELAY_EVENTOS)
    {
        verificar_sensor[indice]();
        indice = ++indice % 2;
        tiempo_anterior = tiempo_actual;
    }
}

void verificar_estado_sensor_boton()
{
    int lectura_boton = digitalRead(sensor_boton.pin);

    if (lectura_boton != sensor_boton.anterior) // Si hay un cambio en el estado
    {
        tiempo_delay_boton = tiempo_actual; // Reiniciamos el temporizador
    }

    if (tiempo_actual - tiempo_delay_boton > TMP_DELAY_BOTON_MILI) // Si no se produjo un cambio por mas de TMP_DELAY_BOTON_MILI (default = 50ms)
    {
        if (lectura_boton != sensor_boton.actual) // Si la lectura nueva != ultima lectura fija
        {
            sensor_boton.actual = lectura_boton; // Guardamos dicha lectura

            if (lectura_boton == LOW)
            {
                // BOTON PASA DE NO APRETADO A APRETADO
            }
            else
            {
                // BOTON PASA DE APRETADO A NO APRETADO
                evento = EVENTO_BOTON;
            }
        }
    }
}

void verificar_estado_sensor_acelerometro()
{
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t rot_x;
    int16_t rot_y;
    int16_t rot_z;

    sensor_mpu6050.mpu.getMotion6(&accel_x, &accel_y, &accel_z, &rot_x, &rot_y, &rot_z);

    if (accel_x > 2048)
    {
        evento = EVENTO_CAIDA;
    }
}

//-------------------------------------------------
// Funcion setup
//-------------------------------------------------
void setup()
{
    start();
}

//-------------------------------------------------
// Funcion loop
//-------------------------------------------------
void loop()
{
    fsm();
}