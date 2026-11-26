#include <WiFi.h>
#include <WiFiUdp.h>
#include "secrets.h"

const char* wifiSsid  = WIFI_SSID;
const char* wifiClave = WIFI_CLAVE;
const int   puertoUdp = PUERTO_UDP;
const unsigned long TIEMPO_SIN_DATOS = 600; // ms sin datos -> frena todo

IPAddress ipFija(IP_ESP32);
IPAddress puerta(IP_GATEWAY);
IPAddress mascara(IP_SUBNET);

// ===== Pines ESP32 -> L298N =====
// Motor de tracción -> En OUT3 y OUT4 (ENB, IN3, IN4)
const int PIN_ENA = 4;
const int PIN_IN1 = 17;
const int PIN_IN2 = 16;

// Motor de dirección -> En OUT1 y OUT2 (ENA, IN1, IN2)
const int PIN_ENB = 19;
const int PIN_IN3 = 18;
const int PIN_IN4 = 5;

const int PIN_LED = 2;

// Configuración PWM
const int FREC_PWM = 500;
const int RES_PWM  = 8;

WiFiUDP udp;
unsigned long ultimoPaquete = 0;

// ===== Variables del mando =====
int joystickIzquierdoX = 0;
int joystickIzquierdoY = 0;
int joystickDerechoX   = 0;
int joystickDerechoY   = 0;
int gatilloL2 = 0;
int gatilloR2 = 0;

int botonX         = 0;
int botonCirculo   = 0;
int botonCuadrado  = 0;
int botonTriangulo = 0;
int botonL1        = 0;
int botonR1        = 0;
int botonShare     = 0;
int botonOptions   = 0;
int botonL3        = 0;
int botonR3        = 0;
int flechaArriba    = 0;
int flechaAbajo     = 0;
int flechaIzquierda = 0;
int flechaDerecha   = 0;
int botonPS        = 0;
int botonTouchpad  = 0;

// ===== Control de motores =====
void moverTraccion(int valor) {
  // Polaridad invertida por software para que L2 vaya hacia adelante
  if (valor > 10) {
    // Adelante (L2)
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, HIGH);
    int pwm = map(valor, 10, 100, 90, 255);
    ledcWrite(PIN_ENA, pwm);
  } else if (valor < -10) {
    // Retroceso (L1)
    digitalWrite(PIN_IN1, HIGH);
    digitalWrite(PIN_IN2, LOW);
    int pwm = map(-valor, 10, 100, 90, 255);
    ledcWrite(PIN_ENA, pwm);
  } else {
    // Detenido
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, LOW);
    ledcWrite(PIN_ENA, 0);
  }
}

void moverDireccion(int valor) {
  if (valor > 15) {
    digitalWrite(PIN_IN3, HIGH);
    digitalWrite(PIN_IN4, LOW);
    ledcWrite(PIN_ENB, 255);
  } else if (valor < -15) {
    digitalWrite(PIN_IN3, LOW);
    digitalWrite(PIN_IN4, HIGH);
    ledcWrite(PIN_ENB, 255);
  } else {
    digitalWrite(PIN_IN3, LOW);
    digitalWrite(PIN_IN4, LOW);
    ledcWrite(PIN_ENB, 0);
  }
}

void usarMando() {
  // LED indicador con botón X
  digitalWrite(PIN_LED, botonX == 1 ? HIGH : LOW);

  // TRACCIÓN: L2 acelera (analógico), L1 retrocede
  int aceleracion = 0;
  if (gatilloL2 > 10) {
    aceleracion = gatilloL2;
  } else if (botonL1 == 1) {
    aceleracion = -85;
  }

  // DIRECCIÓN: Cuadrado = Izquierda (-100), Círculo = Derecha (100)
  int giro = 0;
  if (botonCirculo == 1 && botonCuadrado == 0) {
    giro = 100;
  } else if (botonCuadrado == 1 && botonCirculo == 0) {
    giro = -100;
  }

  moverTraccion(aceleracion);
  moverDireccion(giro);
}

void leerMando() {
  while (udp.parsePacket() > 0) {
    char texto[128];
    int largo = udp.read(texto, 127);
    if (largo <= 0) continue;
    texto[largo] = '\0';

    int valores[22];
    int cantidad = 0;
    char* parte = strtok(texto, ",");
    while (parte != NULL && cantidad < 22) {
      valores[cantidad] = atoi(parte);
      cantidad++;
      parte = strtok(NULL, ",");
    }

    if (cantidad != 22) continue;

    joystickIzquierdoX = valores[0];
    joystickIzquierdoY = valores[1];
    joystickDerechoX   = valores[2];
    joystickDerechoY   = valores[3];
    gatilloL2          = valores[4];
    gatilloR2          = valores[5];
    botonX             = valores[6];
    botonCirculo       = valores[7];
    botonCuadrado      = valores[8];
    botonTriangulo     = valores[9];
    botonL1            = valores[10];
    botonR1            = valores[11];
    botonShare         = valores[12];
    botonOptions       = valores[13];
    botonL3            = valores[14];
    botonR3            = valores[15];
    flechaArriba       = valores[16];
    flechaAbajo        = valores[17];
    flechaIzquierda    = valores[18];
    flechaDerecha      = valores[19];
    botonPS            = valores[20];
    botonTouchpad      = valores[21];

    ultimoPaquete = millis();
  }
}

void ponerTodoACero() {
  joystickIzquierdoX = 0;
  joystickIzquierdoY = 0;
  joystickDerechoX   = 0;
  joystickDerechoY   = 0;
  gatilloL2          = 0;
  gatilloR2          = 0;
  botonX             = 0;
  botonCirculo       = 0;
  botonCuadrado      = 0;
  botonTriangulo     = 0;
  botonL1            = 0;
  botonR1            = 0;
  botonShare         = 0;
  botonOptions       = 0;
  botonL3            = 0;
  botonR3            = 0;
  flechaArriba       = 0;
  flechaAbajo        = 0;
  flechaIzquierda    = 0;
  flechaDerecha      = 0;
  botonPS            = 0;
  botonTouchpad      = 0;
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);

  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);

  ledcAttach(PIN_ENA, FREC_PWM, RES_PWM);
  ledcAttach(PIN_ENB, FREC_PWM, RES_PWM);
  ledcWrite(PIN_ENA, 0);
  ledcWrite(PIN_ENB, 0);

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.config(ipFija, puerta, mascara);
  WiFi.begin(wifiSsid, wifiClave);

  Serial.print("Conectando a ");
  Serial.print(wifiSsid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.localIP());

  udp.begin(puertoUdp);
  Serial.print("Escuchando UDP en puerto ");
  Serial.println(puertoUdp);
}

void loop() {
  leerMando();

  if (millis() - ultimoPaquete > TIEMPO_SIN_DATOS) {
    ponerTodoACero();
  }

  usarMando();
}