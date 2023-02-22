#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <HX711_ADC.h>
#include <AccelStepper.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <EEPROM.h>
#include <Stepper.h>

// Definición de pines de entrada y salida
const int PIN_MOTOR_PASOS_PULSO = 5;
const int PIN_MOTOR_PASOS_DIRECCION = 6;
const int PIN_ENCODER_CLK = 8;
const int PIN_ENCODER_DT = 9;
const int PIN_ENCODER_BTN = 10;
const int PIN_CELDA_CARGA_DT = A0;
const int PIN_CELDA_CARGA_SCK = A1;
const int PIN_BOTON_EMERGENCIA = 12;
const int PIN_BOTON_INICIO_FIN = 13;

// Definición de constantes para el control del motor, celda de carga, LCD y encoder
const int NUMERO_DE_PASOS_POR_REVOLUCION_DEL_MOTOR = 200;
const int MICROSEGUNDOS_ENTRE_PASOS_MOTOR = 200;
const float CALIBRACION_CELDA_DE_CARGA = -22591.0;
const int MAX_LONGITUD_MENSAJE_LCD = 16;

// Definición de variables
bool esBotonInicioFinPresionado = false;
bool esBotonEmergenciaPresionado = false;
float pesoObjeto = 0.0;
bool objetoEnMovimiento = false;
bool direccionMovimiento = true; // true = hacia el inicio, false = hacia el final
bool esInicioMovimiento = false;
int posicionActual = 0;

// Declaración de objetos
LiquidCrystal_I2C lcd(0x27, 16, 2);
HX711_ADC celdaDeCarga;
AccelStepper motorDePasos(AccelStepper::DRIVER, PIN_MOTOR_PASOS_PULSO, PIN_MOTOR_PASOS_DIRECCION);
ClickEncoder encoder(PIN_ENCODER_DT, PIN_ENCODER_CLK, PIN_ENCODER_BTN, 2, true);
Stepper stepper(NUMERO_DE_PASOS_POR_REVOLUCION_DEL_MOTOR, 8, 9, 10, 11);
void setup() {
  // Configuración de pines de entrada y salida
  pinMode(PIN_BOTON_EMERGENCIA, INPUT_PULLUP);
  pinMode(PIN_BOTON_INICIO_FIN, INPUT_PULLUP);

  // Configuración de la comunicación I2C para el LCD
  Wire.begin();
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Inicializando...");

  // Configuración de la celda de carga
  celdaDeCarga.begin(PIN_CELDA_CARGA_DT, PIN_CELDA_CARGA_SCK);
  celdaDeCarga.setCalibration(CALIBRACION_CELDA_DE_CARGA);

  // Configuración
// Declaración de objetos
LiquidCrystal_I2C lcd(0x27, 16, 2);
HX711_ADC celdaDeCarga;
AccelStepper motorDePasos(AccelStepper::DRIVER, PIN_MOTOR_PASOS_PULSO, PIN_MOTOR_PASOS_DIRECCION);
ClickEncoder encoder(PIN_ENCODER_DT, PIN_ENCODER_CLK, PIN_ENCODER_BTN, 2, true);
Stepper stepper(NUMERO_DE_PASOS_POR_REVOLUCION_DEL_MOTOR, 8, 9, 10, 11);

void setup() {
  // Inicialización de objetos y pines
  Wire.begin();
  lcd.init();
  lcd.backlight();
  celdaDeCarga.begin(PIN_CELDA_CARGA_DT, PIN_CELDA_CARGA_SCK);
  celdaDeCarga.setCalibration(CALIBRACION_CELDA_DE_CARGA);
  motorDePasos.setMaxSpeed(1000);
  motorDePasos.setAcceleration(500);
  encoder.setAccelerationEnabled(true);
  encoder.setDoubleClickEnabled(true);
  pinMode(PIN_BOTON_EMERGENCIA, INPUT_PULLUP);
  pinMode(PIN_BOTON_INICIO_FIN, INPUT_PULLUP);

  // Mensajes iniciales en el LCD
  lcd.setCursor(0, 0);
  lcd.print("Peso (g)   Pos.");
  lcd.setCursor(0, 1);
  lcd.print("     0     0");

  // Movimiento inicial del motor
  motorDePasos.moveTo(-NUMERO_DE_PASOS_POR_REVOLUCION_DEL_MOTOR / 2);
  motorDePasos.runToPosition();
  objetoEnMovimiento = true;
  direccionMovimiento = false;
  esInicioMovimiento = true;
}
// Sección 3: Funciones de movimiento y control del motor

// Función para mover el objeto a la posición inicial
void moverObjetoInicio() {
int pasosParaMover = abs(posicionActual);
motorDePasos.setAcceleration(200);
motorDePasos.moveTo(pasosParaMover);
objetoEnMovimiento = true;
direccionMovimiento = true;
esInicioMovimiento = true;
}

// Función para mover el objeto a la posición final
void moverObjetoFin() {
int pasosParaMover = NUMERO_DE_PASOS_POR_REVOLUCION_DEL_MOTOR - abs(posicionActual);
motorDePasos.setAcceleration(200);
motorDePasos.moveTo(-pasosParaMover);
objetoEnMovimiento = true;
direccionMovimiento = false;
esInicioMovimiento = false;
}

// Función para detener el motor
void detenerMotor() {
motorDePasos.stop();
motorDePasos.setSpeed(0);
objetoEnMovimiento = false;
}

// Función para cambiar la dirección del motor
void cambiarDireccionMotor() {
if (direccionMovimiento == true) {
moverObjetoFin();
} else {
moverObjetoInicio();
}
}

// Función para verificar el botón de inicio/fin
void verificarBotonInicioFin() {
bool estadoBoton = digitalRead(PIN_BOTON_INICIO_FIN);
if (estadoBoton == LOW) {
esBotonInicioFinPresionado = true;
} else {
if (esBotonInicioFinPresionado) {
cambiarDireccionMotor();
}
esBotonInicioFinPresionado = false;
}
}

// Función para verificar el botón de emergencia
void verificarBotonEmergencia() {
bool estadoBoton = digitalRead(PIN_BOTON_EMERGENCIA);
if (estadoBoton == LOW) {
esBotonEmergenciaPresionado = true;
} else {
if (esBotonEmergenciaPresionado) {
detenerMotor();
}
esBotonEmergenciaPresionado = false;
}
}

// Función para actualizar la posición del objeto
void actualizarPosicionObjeto() {
long posicion = encoder.getValue();
int pasos = stepper.steps();
posicionActual = map(posicion, -pasos, pasos, -NUMERO_DE_PASOS_POR_REVOLUCION_DEL_MOTOR, NUMERO_DE_PASOS_POR_REVOLUCION_DEL_MOTOR);
}

// Función para actualizar el peso del objeto
void actualizarPesoObjeto() {
pesoObjeto = celdaDeCarga.getData();
}

// Función para actualizar la pantalla LCD
void actualizarLCD() {
lcd.setCursor(0, 0);
lcd.print("Pos: ");
lcd.print(posicionActual);
lcd.print(" ");
lcd.setCursor(0, 1);
lcd.print("Peso: ");
lcd.print(pesoObjeto, 1);
lcd.print(" g");
if (objetoEnMovimiento) {
lcd.print(" M");
if (esInicioMovimiento) {
lcd.print("I");
} else {
lcd.print("F");
}
}
}

// Función de loop
void loop() {
// Verificar el botón de inicio/fin
verificarBotonInicioFin();

// Verificar el botón de