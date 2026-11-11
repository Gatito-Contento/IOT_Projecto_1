#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

// Definición de pines para ESP32 (Módulo LoRa)
#define SS 5
#define RST 14
#define DIO0 2

// Configuración del sensor DHT22
#define PIN_DHT 4      
#define TIPO_DHT DHT22
DHT dht(PIN_DHT, TIPO_DHT);

// Definición del LED indicador en el pin 16
#define PIN_LED 16  

// Control de tiempos (en milisegundos)
unsigned long tiempoUltimaLectura = 0;
const long intervaloLoRa = 3000;      // Lee el sensor y envía por LoRa cada 3 segundos

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Iniciando Sistema: LoRa Rápido (sin ThingSpeak)...");

  // Inicializar periféricos
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  dht.begin();

  // Configurar pines del módulo LoRa
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(915E6)) {
    Serial.println("Error: No se encontró el módulo LoRa. Revisa los cables.");
    while (1);
  }
 
  Serial.println("LoRa listo en 915 MHz.");
}

void loop() {
  unsigned long tiempoActual = millis();
 
  // ==========================================
  // LECTURA Y ENVÍO POR LORA (Cada 3 s)
  // ==========================================
  if (tiempoActual - tiempoUltimaLectura >= intervaloLoRa) {
    tiempoUltimaLectura = tiempoActual;

    float humedad = dht.readHumidity();
    float temperatura = dht.readTemperature();

    // Validar lectura correcta del sensor
    if (isnan(humedad) || isnan(temperatura)) {
      Serial.println("Error al leer el sensor DHT22. Revisa las conexiones.");
      return;
    }

    // Mostrar datos en el Monitor Serie local
    Serial.print("[LoRa] Temp: ");
    Serial.print(temperatura);
    Serial.print(" °C | Humedad: ");
    Serial.print(humedad);
    Serial.println(" %");

    String mensajeAlerta = " [ESTADO: NORMAL]";
    if (temperatura > 25.0) {
      digitalWrite(PIN_LED, HIGH);   // Enciende LED indicador local
      mensajeAlerta = " [ALERTA: UMBRAL SUPERADO]";
      Serial.println("-> ¡ALERTA! Superó los 25°C (LED encendido).");
    } else {
      digitalWrite(PIN_LED, LOW);    // Apaga LED si está por debajo
      Serial.println("-> Estado Normal.");
    }

    // Enviar paquete por radiofrecuencia LoRa
    LoRa.beginPacket();
    LoRa.print("Temp:");
    LoRa.print(temperatura);
    LoRa.print("C, Hum:");
    LoRa.print(humedad);
    LoRa.print("%");
    LoRa.print(mensajeAlerta);
    LoRa.endPacket();
    Serial.println("Paquete LoRa transmitido de forma rápida.");
  }
}
