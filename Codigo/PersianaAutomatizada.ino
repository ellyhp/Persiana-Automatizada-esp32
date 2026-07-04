#define BLYNK_TEMPLATE_ID   "TMPLxxxxxx"
#define BLYNK_TEMPLATE_NAME "Persiana"
#define BLYNK_AUTH_TOKEN    "xxxxxxxxxxxx"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Preferences.h>

Preferences prefs;

// ========== TB6612 ==========
#define AIN1 25
#define AIN2 26
#define PWMA 27

// ========== BOTONES ==========
#define BTN_UP     32
#define BTN_DOWN   13
#define BTN_STOP   12
#define BTN_AUTO   33

// ========== BUZZER ===========
#define BUZZER 4

// ========== PWM ==============
#define PWM_FREQ 1000
#define PWM_RES  8
#define MOTOR_SPEED 180

// ========== WIFI =============
char ssid[] = "TU_RED_WIFI";
char pass[] = "TU_CONTRASEÑA";

// ========== ESTADO GENERAL ===========
bool modoProgramacion = false;
bool modoLimite        = false;
bool persianaAbierta   = false;

unsigned long tiempoMax = 8000;
long          limiteMax = -1;

unsigned long inicioTiempo = 0;
int           signoTiempo  = 1;

long posicionActual = 0;

String ultimoEstado = "";

bool blynkUp   = false;
bool blynkDown = false;
bool blynkAuto = false;

// ========== Tap counter para el botón STOP ==========
#define TAP_INTERVALO 1000
int contadorTapStop = 0;
unsigned long ultimoTapStop = 0;
bool decisionTomada = false;

// ============================================================
void logBoth(String texto) {
  Serial.println(texto);
  Blynk.virtualWrite(V4, texto + "\n");
}

// ============================================================
BLYNK_WRITE(V0) { blynkUp = (param.asInt() == 1); }
BLYNK_WRITE(V1) { blynkDown = (param.asInt() == 1); }

BLYNK_WRITE(V2) {
  if (param.asInt() == 1) {
    logBoth(">>> [BLYNK] STOP");
    if (modoProgramacion) { guardarYSalirProgramacion(); return; }
    if (modoLimite)        { guardarYSalirLimite(); return; }
    registrarTapStop();
  }
}

BLYNK_WRITE(V3) {
  if (param.asInt() == 1) blynkAuto = true;
}

// ============================================================
void setup() {
  Serial.begin(115200);

  prefs.begin("persiana", false);
  tiempoMax = prefs.getULong("tiempo", 8000);
  limiteMax = prefs.getLong("limite", -1);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BTN_UP,   INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_STOP, INPUT_PULLUP);
  pinMode(BTN_AUTO, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  // ===== CORRECCIÓN CORE 3.x =====
  ledcAttach(PWMA, PWM_FREQ, PWM_RES);

  stopMotor();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Blynk.virtualWrite(V4, "clr");

  logBoth("");
  logBoth("=======================");
  logBoth("Sistema iniciado");
  logBoth("=======================");
  logBoth("Tiempo guardado: " + String(tiempoMax / 1000.0) + " s");

  if (limiteMax >= 0) logBoth("Limite guardado: " + String(limiteMax / 1000.0) + " s");
  else logBoth("Sin limite configurado");

  posicionActual = 0;
}

// ============================================================
void loop() {
  Blynk.run();

  detectarTapStopFisico();
  resolverTapStopPendiente();

  if (modoLimite)        { procesarModoLimite();       return; }
  if (modoProgramacion)  { procesarModoProgramacion(); return; }

  bool autoFisico = (digitalRead(BTN_AUTO) == LOW);
  bool autoBlynkEdge = blynkAuto;
  blynkAuto = false;

  static bool autoFisicoAnterior = false;
  bool autoFisicoEdge = (!autoFisicoAnterior && autoFisico);
  autoFisicoAnterior = autoFisico;

  if (autoFisicoEdge || autoBlynkEdge) {
    ejecutarAuto();
  }

  procesarManual();

  delay(100);
}

// ============================================================
bool puedeSubir() {
  if (limiteMax < 0) return true;
  return posicionActual < limiteMax;
}

bool puedeBajar() {
  return posicionActual > 0;
}

// ============================================================
void procesarManual() {

  bool subiendo = (digitalRead(BTN_UP) == LOW) || blynkUp;
  bool bajando  = (digitalRead(BTN_DOWN) == LOW) || blynkDown;

  if (subiendo && puedeSubir()) {
    if (ultimoEstado != "ABRIENDO") { logBoth(""); logBoth(">>> ABRIENDO"); ultimoEstado = "ABRIENDO"; }
    if (inicioTiempo == 0) { inicioTiempo = millis(); signoTiempo = 1; }
    openMotor();
    actualizarPosicionEnVivo();
  }
  else if (bajando && puedeBajar()) {
    if (ultimoEstado != "CERRANDO") { logBoth(""); logBoth(">>> CERRANDO"); ultimoEstado = "CERRANDO"; }
    if (inicioTiempo == 0) { inicioTiempo = millis(); signoTiempo = -1; }
    closeMotor();
    actualizarPosicionEnVivo();
  }
  else if (digitalRead(BTN_STOP) == LOW) {
    stopMotor();
    consolidarPosicion();
    if (ultimoEstado != "STOP") { logBoth(""); logBoth(">>> DETENIDO"); ultimoEstado = "STOP"; }
  }
  else {
    stopMotor();
    consolidarPosicion();
    if ((subiendo && !puedeSubir()) || (bajando && !puedeBajar())) {
      if (ultimoEstado != "LIMITE_TOPE") {
        logBoth(">>> LIMITE ALCANZADO, no se puede mover mas en esa direccion");
        ultimoEstado = "LIMITE_TOPE";
      }
    } else {
      ultimoEstado = "";
    }
  }
}

// ============================================================
void ejecutarAuto() {

  logBoth("");
  logBoth(">>> AUTO");

  if (!persianaAbierta) {
    logBoth("Abriendo");
    unsigned long t0 = millis();
    while (millis() - t0 < tiempoMax) {
      if (!puedeSubir()) break;
      openMotor();
      posicionActual += 50;
      delay(50);
      Blynk.run();
    }
    stopMotor();
    persianaAbierta = true;
  } else {
    logBoth("Cerrando");
    unsigned long t0 = millis();
    while (millis() - t0 < tiempoMax) {
      if (!puedeBajar()) break;
      closeMotor();
      posicionActual -= 50;
      delay(50);
      Blynk.run();
    }
    stopMotor();
    persianaAbierta = false;
  }

  if (posicionActual < 0) posicionActual = 0;
  if (limiteMax >= 0 && posicionActual > limiteMax) posicionActual = limiteMax;

  beep(1);
  delay(1000);
}

// ============================================================
void actualizarPosicionEnVivo() {
  if (inicioTiempo != 0) {
    long transcurrido = millis() - inicioTiempo;
    long delta = signoTiempo * transcurrido;
    long nuevaPos = posicionActual + delta;

    if (signoTiempo == 1 && limiteMax >= 0 && nuevaPos > limiteMax) {
      nuevaPos = limiteMax;
      stopMotor();
    }
    if (signoTiempo == -1 && nuevaPos < 0) {
      nuevaPos = 0;
      stopMotor();
    }

    posicionActual = nuevaPos;
    inicioTiempo = millis();
  }
}

void consolidarPosicion() {
  inicioTiempo = 0;
}

// ============================================================
// ---------- MODO PROGRAMACION ----------
void entrarModoProgramacion() {
  modoProgramacion = true;
  inicioTiempo = 0;
  logBoth("");
  logBoth(">>> MODO PROGRAMACION");
  logBoth("Posicion actual: " + String(posicionActual / 1000.0) + " s");
  beep(1);
}

void procesarModoProgramacion() {

  bool subiendo = ((digitalRead(BTN_UP) == LOW) || blynkUp) && puedeSubir();
  bool bajando  = ((digitalRead(BTN_DOWN) == LOW) || blynkDown) && puedeBajar();

  if (subiendo) {
    if (ultimoEstado != "ARRIBA") { logBoth(""); logBoth(">>> AJUSTANDO ARRIBA (+)"); ultimoEstado = "ARRIBA"; }
    if (inicioTiempo == 0) { inicioTiempo = millis(); signoTiempo = 1; }
    openMotor();
    actualizarPosicionEnVivo();
    mostrarPosicion();
  }
  else if (bajando) {
    if (ultimoEstado != "ABAJO") { logBoth(""); logBoth(">>> AJUSTANDO ABAJO (-)"); ultimoEstado = "ABAJO"; }
    if (inicioTiempo == 0) { inicioTiempo = millis(); signoTiempo = -1; }
    closeMotor();
    actualizarPosicionEnVivo();
    mostrarPosicion();
  }
  else {
    stopMotor();
    consolidarPosicion();
    ultimoEstado = "";
  }

  if (digitalRead(BTN_STOP) == LOW) {
    guardarYSalirProgramacion();
    delay(1000);
  }
}

void guardarYSalirProgramacion() {
  stopMotor();
  consolidarPosicion();

  tiempoMax = (unsigned long) posicionActual;
  prefs.putULong("tiempo", tiempoMax);

  logBoth("");
  logBoth("CONFIGURACION GUARDADA");
  logBoth("Tiempo: " + String(tiempoMax / 1000.0) + " s");

  beep(2);

  inicioTiempo = 0;
  modoProgramacion = false;
  ultimoEstado = "";
}

void mostrarPosicion() {
  static long ultimo = -999999;
  long segundos = posicionActual / 1000;
  if (segundos != ultimo) {
    ultimo = segundos;
    logBoth("Posicion: " + String(segundos) + " s");
  }
}

// ============================================================
// ---------- MODO LIMITE ----------
void entrarModoLimite() {
  modoLimite = true;
  inicioTiempo = 0;
  logBoth("");
  logBoth(">>> MODO LIMITE");
  logBoth("Posicion actual: " + String(posicionActual / 1000.0) + " s");
  beep(1);
}

void procesarModoLimite() {

  bool subiendo = (digitalRead(BTN_UP) == LOW) || blynkUp;
  bool bajando  = ((digitalRead(BTN_DOWN) == LOW) || blynkDown) && (posicionActual > 0);

  if (subiendo) {
    if (ultimoEstado != "ARRIBA_LIM") { logBoth(""); logBoth(">>> AJUSTANDO LIMITE ARRIBA (+)"); ultimoEstado = "ARRIBA_LIM"; }
    if (inicioTiempo == 0) { inicioTiempo = millis(); signoTiempo = 1; }
    openMotor();
    actualizarPosicionLibre();
    mostrarPosicion();
  }
  else if (bajando) {
    if (ultimoEstado != "ABAJO_LIM") { logBoth(""); logBoth(">>> AJUSTANDO LIMITE ABAJO (-)"); ultimoEstado = "ABAJO_LIM"; }
    if (inicioTiempo == 0) { inicioTiempo = millis(); signoTiempo = -1; }
    closeMotor();
    actualizarPosicionLibre();
    mostrarPosicion();
  }
  else {
    stopMotor();
    consolidarPosicion();
    ultimoEstado = "";
  }

  if (digitalRead(BTN_STOP) == LOW) {
    guardarYSalirLimite();
    delay(1000);
  }
}

void actualizarPosicionLibre() {
  if (inicioTiempo != 0) {
    long transcurrido = millis() - inicioTiempo;
    long nuevaPos = posicionActual + signoTiempo * transcurrido;
    if (nuevaPos < 0) { nuevaPos = 0; stopMotor(); }
    posicionActual = nuevaPos;
    inicioTiempo = millis();
  }
}

void guardarYSalirLimite() {
  stopMotor();
  consolidarPosicion();

  limiteMax = posicionActual;
  prefs.putLong("limite", limiteMax);

  logBoth("");
  logBoth("LIMITE GUARDADO");
  logBoth("Limite: " + String(limiteMax / 1000.0) + " s");

  beep(2);

  inicioTiempo = 0;
  modoLimite = false;
  ultimoEstado = "";
}

// ============================================================
// ---------- Detección de taps en STOP ----------
void detectarTapStopFisico() {
  static bool ultimoEstadoBtn = HIGH;
  bool actual = digitalRead(BTN_STOP);

  if (ultimoEstadoBtn == HIGH && actual == LOW && !modoProgramacion && !modoLimite) {
    registrarTapStop();
  }

  ultimoEstadoBtn = actual;
}

void registrarTapStop() {
  unsigned long ahora = millis();

  if (ahora - ultimoTapStop > TAP_INTERVALO) contadorTapStop = 1;
  else contadorTapStop++;

  ultimoTapStop = ahora;
  decisionTomada = false;
}

void resolverTapStopPendiente() {
  if (contadorTapStop == 0 || decisionTomada) return;
  if (modoProgramacion || modoLimite) { contadorTapStop = 0; return; }

  unsigned long ahora = millis();

  if (ahora - ultimoTapStop > TAP_INTERVALO) {
    if (contadorTapStop == 2)   entrarModoLimite();
    else if (contadorTapStop >= 3) entrarModoProgramacion();

    contadorTapStop = 0;
    decisionTomada = true;
  }
}

// ============================================================
// ===== CORRECCIÓN CORE 3.x: ledcWrite usa el pin, no el canal =====
void openMotor()  { digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);  ledcWrite(PWMA, MOTOR_SPEED); }
void closeMotor() { digitalWrite(AIN1, LOW);  digitalWrite(AIN2, HIGH); ledcWrite(PWMA, MOTOR_SPEED); }
void stopMotor()  { digitalWrite(AIN1, LOW);  digitalWrite(AIN2, LOW);  ledcWrite(PWMA, 0); }

void beep(int veces) {
  for (int i = 0; i < veces; i++) { digitalWrite(BUZZER, HIGH); delay(150); digitalWrite(BUZZER, LOW); delay(150); }
}
}
