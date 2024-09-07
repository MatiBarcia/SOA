#include <Wire.h>
#include <MPU6050.h>

#define PIN_BOTON 2
#define PIN_BUZZER 11
#define PIN_LED 13

enum estados
{
    EMBEBIDO_REPOSO,
    EMBEBIDO_ALERTA_SONANDO,
    EMBEBIDO_ALERTA_FINALIZADA
};

estados estado_actual;

//-------------------------------------------------
// Estructura de sensores
//-------------------------------------------------

typedef struct sensor_boton
{
    int pin;
    int actual;
    int anterior;
} sensor_boton;

typedef struct sensor_mpu6050
{
    MPU6050 mpu;
} sensor_mpu6050;

//-------------------------------------------------
// Estructura de actuadores
//-------------------------------------------------

typedef struct actuador_buzzer
{
    int pin;
} actuador_buzzer;

typedef struct actuador_led
{
    int pin;
    int estado;
} actuador_led;

sensor_boton boton;
sensor_mpu6050 mpu6050;
actuador_buzzer buzzer;
actuador_led led;

unsigned long tiempo_anterior;
unsigned long tiempo_actual;



//-------------------------------------------------
// Funcion inicial
//-------------------------------------------------
void start()
{
    Serial.begin(9600);

    // Setup sensores
    boton.pin = PIN_BOTON;
    pinMode(boton.pin, INPUT_PULLUP);
    boton.anterior = LOW;

    mpu6050.mpu.initialize(ACCEL_FS::A16G, GYRO_FS::G2000DPS);

    // Setup actuadores
    buzzer.pin = PIN_BUZZER;
    pinMode(buzzer.pin, OUTPUT);
    
    led.pin = PIN_LED;
    pinMode(led.pin, OUTPUT);
    led.estado = LOW;
    
    // Se inicializa el temporizador
    tiempo_anterior = millis();
}

//-------------------------------------------------
// Funcion de la maquina de estados
//-------------------------------------------------
void fsm()
{
    tomar_eventos();

    switch (estado_actual)
    {
        case EMBEBIDO_REPOSO:
            // SI SE PULSA EL BOTON O SE DETECTA UNA CAIDA, CAMBIA DE ESTADO A EMBEBIDO_ALERTA_SONANDO
            break;
        case EMBEBIDO_ALERTA_SONANDO:
            // SI SE PULSA EL BOTON, CAMBIA DE ESTADO A EMBEBIDO_ALERTA_FINALIZADA
            break;
        case EMBEBIDO_ALERTA_FINALIZADA:
            break;
        default:
            break;
    }
}

//-------------------------------------------------
// Funciones definidas por nosotros
//-------------------------------------------------
void tomar_eventos()
{
    
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