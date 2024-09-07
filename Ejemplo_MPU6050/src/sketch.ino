#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

float dt = 0.01;         // Intervalo de tiempo entre muestras (ajusta según tus necesidades)
float aceleracionX = 0;  // Aceleración inicial en m/s² en el eje X
float aceleracionY = 0;  // Aceleración inicial en m/s² en el eje Y
float velocidadX = 0;    // Velocidad inicial en cm/s en el eje X
float velocidadY = 0;    // Velocidad inicial en cm/s en el eje Y
float posicionX = 0;     // Posición inicial en cm en el eje X
float posicionY = 0;     // Posición inicial en cm en el eje Y
float offsetX = 0;       // Offset para calibración del acelerómetro en el eje X
float offsetY = 0;       // Offset para calibración del acelerómetro en el eje Y
bool calibrado = false;   // Variable para controlar si se realizó la calibración
unsigned long previousMillis = 0;
const long interval = 1000;  // Intervalo de tiempo de 1 segundo entre actualizaciones
bool robotDetenido = false;
const float umbralAceleracion = 0.01;  // Umbral de aceleración para considerar que el robot se ha detenido (ajusta según tus necesidades)

void setup() {
  Wire.begin();
  mpu.initialize();
  Serial.begin(9600);  // Inicia la comunicación con el monitor serial
  // Realiza la calibración al inicio del código
  calibrarAcelerometro();
  calibrado = true;
  // Configura otros pines o configuraciones según tus necesidades
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);
    
    // Calcula la aceleración en los ejes X e Y y aplica la calibración
    float aceleracionX = (((float)ax / 16384.0)*9.7786) - offsetX; // 16384.0 es la sensibilidad del acelerómetro en LSB/g
    float aceleracionY = (((float)ay / 16384.0)*9.7786) - offsetY;
    
    // Convierte la aceleración a cm/s²
    float aceleracionCmS2X = aceleracionX * 100;
    float aceleracionCmS2Y = aceleracionY * 100;
    
    // Guarda la posición anterior antes de actualizar
    float posicionXAnterior = posicionX;
    float posicionYAnterior = posicionY;
    
    // Actualiza la velocidad en cm/s usando el método de Euler en los ejes X e Y
    velocidadX += aceleracionCmS2X * dt;
    velocidadY += aceleracionCmS2Y * dt;

    // Si el robot se ha detenido, establece la velocidad en cero
    if (abs(aceleracionX) < umbralAceleracion && abs(aceleracionY) < umbralAceleracion) {
      velocidadX = 0;
      velocidadY = 0;
      if (!robotDetenido) {
        robotDetenido = true;
        Serial.println("El robot se ha detenido por completo.");
        Serial.print("Última Posición (X, Y): (");
        Serial.print(posicionXAnterior);
        Serial.print(", ");
        Serial.print(posicionYAnterior);
        Serial.println(") cm");
      }
    } else {
      robotDetenido = false;
    }
    
    // Actualiza la posición en cm usando el método de Euler en los ejes X e Y
    posicionX += velocidadX * dt;
    posicionY += velocidadY * dt;

    // Imprime los datos en el monitor serial
    Serial.print("Aceleración X: ");
    Serial.print(aceleracionCmS2X);  // Aceleración en cm/s² en el eje X
    Serial.print(" cm/s^2, ");
    Serial.print("Velocidad X: ");
    Serial.print(velocidadX);  // Velocidad en cm/s en el eje X
    Serial.print(" cm/s, ");
    Serial.print("Posición X: ");
    Serial.print(posicionX);  // Posición en cm en el eje X
    Serial.print(" cm, ");
    
    Serial.print("Aceleración Y: ");
    Serial.print(aceleracionCmS2Y);  // Aceleración en cm/s² en el eje Y
    Serial.print(" cm/s^2, ");
    Serial.print("Velocidad Y: ");
    Serial.print(velocidadY);  // Velocidad en cm/s en el eje Y
    Serial.print(" cm/s, ");
    Serial.print("Posición Y: ");
    Serial.print(posicionY);  // Posición en cm en el eje Y
    Serial.println(" cm");
  }
  
  // Realiza otras acciones o controla los motores según la velocidad y posición calculadas
  // ...
}

void calibrarAcelerometro() {
  if (!calibrado) {
    int16_t ax, ay, az;
    int numSamples = 1000;  // Número de muestras para la calibración
    for (int i = 0; i < numSamples; i++) {
      mpu.getAcceleration(&ax, &ay, &az);
      offsetX += ((float)ax / 16384.0);
      offsetY += ((float)ay / 16384.0);
      delay(1);
    }
    offsetX /= numSamples;
    offsetY /= numSamples;
  }
}
