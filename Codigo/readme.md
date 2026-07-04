# Persiana Automatizada — ESP32 + Blynk

Control de persiana motorizada vía botones físicos y app Blynk IoT, con protección de límite mecánico y memoria persistente.

---

## ¿Qué hace?

- **Control manual**: el motor sube o baja mientras se mantiene presionado el botón (físico o app).
- **Control automático**: un toque en Auto ejecuta el ciclo completo de apertura o cierre.
- **Modo Programación**: define cuánto dura el recorrido completo que usa Auto.
- **Modo Límite**: define el tope máximo de apertura para proteger el mecanismo.
- **Logs en tiempo real**: todo lo que normalmente va al Monitor Serie también aparece en el Terminal de la app.

---

## Hardware

| Componente | Pin ESP32 |
|---|---|
| TB6612 AIN1 | GPIO 25 |
| TB6612 AIN2 | GPIO 26 |
| TB6612 PWMA | GPIO 27 |
| Botón Subir | GPIO 32 |
| Botón Bajar | GPIO 33 |
| Botón Stop | GPIO 12 |
| Botón Auto | GPIO 13 |
| Buzzer | GPIO 4 |

---

## Dependencias

- [Blynk](https://github.com/blynkkk/blynk-library) — control remoto vía app
- `Preferences.h` — incluida en el core de ESP32, guarda la configuración en flash

> Requiere **ESP32 Arduino core 3.x**. Si usas una versión anterior, reemplaza `ledcAttach()` por `ledcSetup()` + `ledcAttachPin()`.

---

## Configuración rápida

1. Crea un Template en [blynk.cloud](https://blynk.cloud) con 5 pines virtuales (V0–V4).
2. Reemplaza en el código:
```cpp
   #define BLYNK_TEMPLATE_ID   "TMPLxxxxxx"
   #define BLYNK_TEMPLATE_NAME "Persiana"
   #define BLYNK_AUTH_TOKEN    "xxxxxxxxxxxx"
   char ssid[] = "TU_RED_WIFI";
   char pass[] = "TU_CONTRASEÑA";
```
3. Compila y carga al ESP32.

---

## Pines virtuales Blynk

| Pin | Función | Modo widget |
|---|---|---|
| V0 | Abrir (Subir) | Switch |
| V1 | Cerrar (Bajar) | Switch |
| V2 | Stop / entrar modos | Push |
| V3 | Auto | Push |
| V4 | Terminal de logs | Terminal |

---

## Modos especiales (botón Stop)

| Toques en Stop | Resultado |
|---|---|
| 1 toque | Detiene el motor |
| 2 toques rápidos | Entra a modo **Límite** |
| 3 toques rápidos | Entra a modo **Programación** |
| 1 toque (dentro de un modo) | Guarda y sale |

> En físico también se puede entrar a Programación manteniendo Stop presionado 3 segundos.

---

## Estructura del código
setup()
└── Inicializa pines, PWM, WiFi y Blynk
loop()
├── Blynk.run()              → mantiene la conexión activa
├── detectarTapStopFisico()  → detecta flancos en el botón físico
├── resolverTapStopPendiente()→ decide modo según cantidad de taps
├── procesarModoLimite()     → ajuste del tope máximo
├── procesarModoProgramacion()→ ajuste del tiempo de recorrido
├── ejecutarAuto()           → ciclo completo abrir/cerrar
└── procesarManual()         → control momentáneo subir/bajar
Funciones de motor:
├── openMotor()   → AIN1=HIGH, AIN2=LOW,  PWM=MOTOR_SPEED
├── closeMotor()  → AIN1=LOW,  AIN2=HIGH, PWM=MOTOR_SPEED
└── stopMotor()   → AIN1=LOW,  AIN2=LOW,  PWM=0

---

## Notas

- La posición se calcula por tiempo transcurrido (no hay sensor físico). Si el ESP32 se reinicia con la persiana en una posición distinta de cerrada, recalibra bajándola del todo manualmente.
- Los valores de `tiempoMax` y `limiteMax` se guardan en flash y persisten entre reinicios.
- Durante el ciclo Auto, `Blynk.run()` se llama dentro del bucle para mantener la conexión activa.
