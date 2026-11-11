# 📡 Monitor de Temperatura y Humedad con LoRa

¿Quieres saber qué temperatura y humedad hay en un lugar lejano **sin necesitar WiFi ni Internet**? Este proyecto hace justo eso: dos ESP32 hablan entre sí por radio usando **LoRa**. Uno mide, el otro recibe y avisa. 🌡️💧

Si la temperatura pasa de **25 °C**, se prende un LED de alerta en el emisor **y también en el receptor**. Si el emisor deja de enviar datos, el receptor se da cuenta y te lo dice.

---

## 🧭 ¿Cómo funciona?

```
┌──────────────────────┐        📻 LoRa 915 MHz        ┌──────────────────────┐
│     NODO EMISOR      │  ─────────────────────────►   │    NODO RECEPTOR     │
│                      │                               │                      │
│  ESP32 + DHT22       │   "Temp:26.50C, Hum:60.20%    │  ESP32 + LED         │
│  LED en GPIO 16      │    [ALERTA: UMBRAL SUPERADO]" │  LED en GPIO 16      │
│  Módulo LoRa         │                               │  Módulo LoRa         │
└──────────────────────┘                               └──────────────────────┘
   Mide cada 3 segundos                                  Escucha todo el tiempo
   Prende LED si pasa de 25 °C                           Copia el LED y vigila la señal
```

**El emisor:**
1. Lee el sensor DHT22 cada 3 segundos.
2. Si la lectura falla, lo avisa y no envía nada.
3. Si la temperatura pasa de 25 °C, prende su LED y marca el mensaje como alerta.
4. Manda el mensaje por LoRa.

**El receptor:**
1. Está siempre escuchando.
2. Cuando llega un mensaje, muestra la temperatura, la humedad, el estado y la fuerza de la señal (RSSI).
3. Prende o apaga su LED según el estado que le llegó.
4. Si pasan más de **10 segundos** sin recibir nada, avisa por el Monitor Serie que se perdió la señal y apaga el LED por seguridad.

---

## ✨ Lo bueno de este proyecto

- 📡 Comunicación inalámbrica de largo alcance y bajo consumo.
- 🚫 Cero Internet, cero WiFi, cero nube.
- 💡 Alerta visual con LED en ambos lados.
- 🛡️ El receptor revisa que el mensaje venga bien armado; si no, lo ignora sin romperse.
- 📶 Muestra el RSSI para que veas qué tan buena es la señal.
- ⏱️ El código no usa `delay()`, trabaja con `millis()` y por eso nunca se queda "congelado".

---

## 🛒 ¿Qué necesito?

| Cantidad | Componente |
|----------|------------|
| 2 | ESP32 (cualquier DevKit sirve) |
| 2 | Módulos LoRa SX1276 o SX1278 de 915 MHz |
| 1 | Sensor DHT22 |
| 2 | LEDs con resistencia de 220 Ω |
| 2 | Antenas para 915 MHz |
| — | Cables jumper, protoboard y fuente de alimentación |

> ⚠️ **Ojo con la frecuencia:** en América se usa **915 MHz**; en Europa, **868 MHz**. Lo importante es que **los dos módulos usen la misma**.

> ⚠️ **Nunca enciendas un módulo LoRa sin antena conectada**, podrías dañarlo.

---

## 🔌 Conexiones

### Nodo emisor

| Pin del ESP32 | Va conectado a |
|---------------|----------------|
| GPIO 5  | LoRa NSS (SS) |
| GPIO 14 | LoRa RST |
| GPIO 2  | LoRa DIO0 |
| GPIO 4  | DHT22 (pin DATA) |
| GPIO 16 | LED (con su resistencia) |
| 3.3 V   | VCC del LoRa y del DHT22 |
| GND     | GND del LoRa, del DHT22 y del LED |

> Los pines SCK, MISO y MOSI del módulo LoRa van a los pines SPI por defecto del ESP32 (GPIO 18, 19 y 23).

### Nodo receptor

| Pin del ESP32 | Va conectado a |
|---------------|----------------|
| GPIO 5  | LoRa NSS (SS) |
| GPIO 14 | LoRa RST |
| GPIO 2  | LoRa DIO0 |
| GPIO 16 | LED (con su resistencia) |
| 3.3 V   | VCC del LoRa |
| GND     | GND del LoRa y del LED |

> Si tu DHT22 viene suelto (sin placa), pon una resistencia de **10 kΩ** entre DATA y VCC.

---

## 🧰 Preparar el entorno

1. Instala el **Arduino IDE** (la versión 2.x va perfecto).
2. Agrega el soporte para ESP32: en *Archivo → Preferencias → Gestor de URLs adicionales de tarjetas* pega esto:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Ve a *Herramientas → Placa → Gestor de tarjetas*, busca **esp32** e instala el paquete de **Espressif Systems**.
4. En el *Gestor de librerías* instala estas tres:

| Librería | Autor | Para qué sirve |
|----------|-------|----------------|
| **LoRa** | Sandeep Mistry | Hablar con el módulo LoRa |
| **DHT sensor library** | Adafruit | Leer el DHT22 |
| **Adafruit Unified Sensor** | Adafruit | La necesita la librería del DHT |

---

## 🚀 Ponerlo a andar

1. Selecciona la placa **ESP32 Dev Module** y el puerto correcto.
2. Sube el sketch del **emisor** a un ESP32.
3. Sube el sketch del **receptor** al otro.
4. Abre el **Monitor Serie a 115200 baudios** en cada uno.
5. Calienta el sensor con la mano o un secador (con cuidado 😅) y mira cómo salta la alerta.

### Ajustes rápidos

**Cambiar los pines** (están al inicio de cada sketch):

```cpp
#define SS      5
#define RST     14
#define DIO0    2
#define PIN_DHT 4     // solo en el emisor
#define PIN_LED 16
```

**Cambiar el umbral de alerta** (en el emisor):

```cpp
if (temperatura > 25.0) {   // cambia 25.0 por la temperatura que quieras
```

---

## 📦 ¿Cómo se ve el mensaje por radio?

Es texto simple, fácil de leer en el Monitor Serie:

```
Temp:<valor>C, Hum:<valor>% [ESTADO: NORMAL]
Temp:<valor>C, Hum:<valor>% [ALERTA: UMBRAL SUPERADO]
```

Por ejemplo:

```
Temp:26.50C, Hum:60.20% [ALERTA: UMBRAL SUPERADO]
```

El receptor busca las palabras clave `Temp:`, `Hum:`, `[ESTADO:` y `[ALERTA:` para sacar los números. Si algo no está en su lugar, simplemente descarta el mensaje.

---

## 🖥️ Así se ve en el Monitor Serie

**Emisor:**
```
Iniciando Sistema: LoRa Rápido (sin ThingSpeak)...
LoRa listo en 915 MHz.
[LoRa] Temp: 24.30 °C | Humedad: 58.10 %
-> Estado Normal.
Paquete LoRa transmitido de forma rápida.
[LoRa] Temp: 26.50 °C | Humedad: 60.20 %
-> ¡ALERTA! Superó los 25°C (LED encendido).
Paquete LoRa transmitido de forma rápida.
```

**Receptor:**
```
Iniciando RECEPTOR LoRa...
Receptor LoRa listo en 915 MHz. Esperando paquetes...
Paquete recibido (RSSI: -48 dBm)
Contenido: Temp:26.50C, Hum:60.20% [ALERTA: UMBRAL SUPERADO]
  -> Temp: 26.50 °C | Hum: 60.20 % | Estado: ALERTA
  -> LED del receptor ENCENDIDO (alerta recibida).
```

---

## 🔧 Si algo sale mal

| Qué pasa | Probable causa | Qué hacer |
|----------|----------------|-----------|
| `Error: No se encontró el módulo LoRa` | Cables mal puestos o sin alimentación | Revisa SS, RST y DIO0, y que el VCC sea de 3.3 V |
| El receptor no recibe nada | Los módulos usan frecuencias distintas | Ambos deben tener `LoRa.begin(915E6)` |
| Señal débil o paquetes que van y vienen | Antena suelta o nodos muy lejos | Conecta bien la antena y acércalos para probar |
| `Error al leer el sensor DHT22` | Falta la resistencia pull-up o hay un cable suelto | Pon una resistencia de 10 kΩ entre DATA y VCC |
| Mensajes con "formato inesperado" | Los dos sketches no son de la misma versión | Sube emisor y receptor desde este mismo proyecto |

---

## 💡 Ideas para seguir mejorándolo

- [ ] Que el receptor responda con un **ACK** para confirmar que llegó el mensaje.
- [ ] Mandar los datos en **binario** para gastar menos tiempo en el aire.
- [ ] Usar **deep sleep** en el emisor para que dure mucho más con batería.
- [ ] Agregar un **checksum** para detectar mensajes dañados.
- [ ] Sumar otro sensor (presión, luz, gas...).
- [ ] Conectar un tercer nodo con WiFi para ver los datos en una página web.

---

> ⭐ Si te sirvió este proyecto, regálale una estrella al repositorio.
