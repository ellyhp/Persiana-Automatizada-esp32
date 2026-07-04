Persiana Automatizada con ESP32 + Blynk


Control inteligente de persiana motorizada vía botones físicos y app móvil, con protección de límite mecánico y memoria persistente.



<p align="center">
  <img src="images/portada.jpg" alt="Persiana Automatizada" width="700"/>
</p>
<p align="center">
  <a href="https://www.instagram.com/ellygmr"><img src="https://img.shields.io/badge/Instagram-@ellygmr-E4405F?style=for-the-badge&logo=instagram&logoColor=white"/></a>
  <a href="https://www.tiktok.com/@ellygmr"><img src="https://img.shields.io/badge/TikTok-@ellygmr-000000?style=for-the-badge&logo=tiktok&logoColor=white"/></a>
  <a href="https://www.youtube.com/@ellygmr"><img src="https://img.shields.io/badge/YouTube-@ellygmr-FF0000?style=for-the-badge&logo=youtube&logoColor=white"/></a>
  <a href="https://www.facebook.com/ellygmr"><img src="https://img.shields.io/badge/Facebook-@ellygmr-1877F2?style=for-the-badge&logo=facebook&logoColor=white"/></a>
</p>

📹 Video del proyecto


🎬 Próximamente — El video con la explicación completa, montaje y demostración estará disponible en mis redes sociales. ¡Sígueme para no perdértelo!




🖨️ Archivos STL para impresión 3D

Los soportes y carcasas del proyecto están disponibles en MakerWorld:


🔗 Descargar archivos STL en MakerWorld (próximamente — enlace en actualización)




✨ ¿Qué hace este sistema?


Control manual — el motor sube o baja mientras se mantiene presionado el botón, físico o desde la app. Al soltar, se detiene.
Control automático — un toque en Auto ejecuta el ciclo completo de apertura o cierre según el tiempo configurado.
Modo Programación — define cuánto dura el recorrido completo que usa Auto, ajustable subiendo y bajando la persiana manualmente.
Modo Límite — define el tope máximo de apertura para proteger el mecanismo y el motor.
Logs en tiempo real — todo lo que sucede en el sistema se refleja en el Terminal de la app Blynk, igual que el Monitor Serie del IDE.
Memoria persistente — el tiempo y el límite configurados se guardan en la flash del ESP32 y sobreviven reinicios.



⚡ Descripción del circuito

El cerebro del sistema es un ESP32 DevKit que se encarga de leer los botones, gestionar la lógica y comunicarse con la app vía WiFi.

El motor DC de 35 RPM a 12V es controlado por el driver TB6612FNG, que actúa como intermediario entre el ESP32 (que trabaja a 3.3V) y el motor (que requiere 12V y corriente más alta). El driver recibe tres señales del ESP32: AIN1 y AIN2 para definir la dirección de giro, y PWMA para controlar la velocidad mediante PWM.

La alimentación del sistema viene de una sola fuente de 12V, que alimenta directamente el motor a través del driver. El convertidor DC-DC LM2596 toma esos mismos 12V y los reduce a 5V estables para alimentar el ESP32, evitando necesitar una segunda fuente de alimentación.

Los cuatro pulsadores (Subir, Bajar, Stop, Auto) se conectan a pines GPIO del ESP32 usando la resistencia pull-up interna, sin necesidad de resistencias externas. El buzzer activo emite señales sonoras de confirmación cuando se guarda una configuración o se entra a un modo especial.

Fuente 12V
    │
    ├──► VM del TB6612FNG ──► Motor DC 35RPM 12V
    │
    └──► LM2596 (step-down) ──► 5V ──► ESP32 DevKit
                                           │
                              GPIO 25 ──► AIN1 (TB6612)
                              GPIO 26 ──► AIN2 (TB6612)
                              GPIO 27 ──► PWMA (TB6612)
                              GPIO 32 ──► Botón Subir
                              GPIO 33 ──► Botón Bajar
                              GPIO 12 ──► Botón Stop
                              GPIO 13 ──► Botón Auto
                              GPIO  4 ──► Buzzer


🛒 Lista de componentes y presupuesto (AliExpress 2026)

ComponenteEspecificaciónUSDMXN aprox.ESP32 DevKit V1WiFi + Bluetooth, dual core$4.50 – $7.50$88 – $146Driver TB6612FNGDual H-bridge, 1.2A, hasta 15V$0.80 – $1.50$16 – $29Convertidor DC-DC LM2596Step-down ajustable 1.5V–35V$0.50 – $1.20$10 – $23Motor DC 12V 35RPMMotor de corriente continua con reductora$3.00 – $12.00$58 – $234Fuente de alimentación 12VMínimo 1–2A$3.50 – $8.00$68 – $1564 pulsadoresMomentáneos, tipo tact switch$0.50 – $1.00$10 – $20Buzzer activo3–5V, señal continua$0.30 – $0.80$6 – $16Cables y protoboard / PCBConexiones y soporte$1.50 – $3.00$29 – $58TOTAL$14.60 – $35$285 – $682


💡 El LM2596 elimina la necesidad de una segunda fuente: toma los 12V del motor y los reduce a 5V para el ESP32. Una sola fuente alimenta todo el sistema.

💱 Precios orientativos con envío estándar gratuito desde AliExpress. Tipo de cambio: ~$19.50 MXN/USD.




📱 Pines virtuales Blynk

PinWidgetModoFunciónV0ButtonSwitchAbrir (subir) — motor activo mientras se mantieneV1ButtonSwitchCerrar (bajar) — motor activo mientras se mantieneV2ButtonPushStop / entrar a modos de configuraciónV3ButtonPushAuto — ciclo completo abrir/cerrarV4Terminal—Log del sistema en tiempo real


🎮 Modos especiales (botón Stop)

Toques en StopResultado1 toqueDetiene el motor2 toques rápidos (< 1 seg)Entra a modo Límite máximo de apertura3 toques rápidos (< 1 seg)Entra a modo Programación del ciclo AutoMantener 3 segundos (físico)Entra a modo Programación1 toque (dentro de un modo)Guarda la configuración y sale


📁 Estructura del repositorio

Persiana-Automatizada-esp32/
│
├── 📁 Codigo/
│   └── persiana_esp32.ino          ← Código fuente completo para Arduino IDE
│
├── 📁 Diagrama/
│   └── Persiana_bb.png             ← Diagrama de conexiones (Fritzing)
│
├── 📁 images/
│   ├── portada.jpg                 ← Foto principal del proyecto montado
│   ├── instalacion_01.jpg          ← Fotos del proceso de instalación
│   ├── instalacion_02.jpg
│   └── ...                         ← Más fotos de referencia
│
├── 📄 README.md                    ← Este archivo
├── 📄 manual_persiana.pdf          ← Manual de uso: botones, programación y límite
└── 📄 manual_blynk_persiana.pdf    ← Manual de configuración de la app Blynk


🔧 Instalación rápida


Instala Arduino IDE y agrega el soporte para ESP32 (core 3.x de Espressif).
Instala la librería Blynk desde el Gestor de Librerías.
Crea un Template en blynk.cloud con los 5 pines virtuales (V0–V4).
Edita las primeras líneas del código con tus credenciales:


cpp#define BLYNK_TEMPLATE_ID   "TMPLxxxxxx"
#define BLYNK_TEMPLATE_NAME "Persiana"
#define BLYNK_AUTH_TOKEN    "xxxxxxxxxxxx"
char ssid[] = "TU_RED_WIFI";
char pass[] = "TU_CONTRASEÑA";


Compila y carga al ESP32.
Sigue el manual_blynk_persiana.pdf para configurar los widgets en la app.



⚠️ Requiere ESP32 Arduino core 3.x. Si usas una versión anterior (2.x), reemplaza ledcAttach() por ledcSetup() + ledcAttachPin().




📚 Documentación incluida

ArchivoContenidomanual_persiana.pdfUso completo del sistema: botones físicos, modo Programación y modo Límitemanual_blynk_persiana.pdfConfiguración paso a paso de Blynk Cloud y la app móvil


🤝 Créditos

Proyecto desarrollado por @ellygmr. Gracias a la comunidad maker y a Bambu Lab por hacer posibles proyectos como este.

Si lo construyes, ¡me encantaría verlo! Etiquétame en cualquiera de mis redes 👇

<p align="center">
  <a href="https://www.instagram.com/ellygmr">📸 Instagram</a> •
  <a href="https://www.tiktok.com/@ellygmr">🎵 TikTok</a> •
  <a href="https://www.youtube.com/@ellygmr">▶️ YouTube</a> •
  <a href="https://www.facebook.com/ellygmr">👥 Facebook</a>
</p>
