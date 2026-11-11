#include <SPI.h>
#include <LoRa.h>

// Definición de pines para ESP32 (Módulo LoRa) - RECEPTOR
#define SS 5
#define RST 14
#define DIO0 2

// LED indicador local del receptor (para replicar el estado del emisor)
#define PIN_LED 16

// Control de tiempo para detectar pérdida de señal
unsigned long tiempoUltimoPaquete = 0;
const long tiempoTimeout = 10000;     // Si pasan 10 s sin paquetes, se considera pérdida de señal
bool senalPerdida = false;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Iniciando RECEPTOR LoRa...");

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  // Configurar pines del módulo LoRa
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(915E6)) {
    Serial.println("Error: No se encontró el módulo LoRa. Revisa los cables.");
    while (1);
  }

  Serial.println("Receptor LoRa listo en 915 MHz. Esperando paquetes...");
}

void loop() {
  // ==========================================
  // TAREA 1: ESCUCHAR PAQUETES LORA
  // ==========================================
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String mensaje = "";

    // Leer todos los bytes del paquete
    while (LoRa.available()) {
      mensaje += (char)LoRa.read();
    }

    tiempoUltimoPaquete = millis();
    senalPerdida = false;

    // Mostrar RSSI (calidad de señal) para diagnóstico
    Serial.print("Paquete recibido (RSSI: ");
    Serial.print(LoRa.packetRssi());
    Serial.println(" dBm)");
    Serial.println("Contenido: " + mensaje);

    // ==========================================
    // TAREA 2: PARSEAR EL MENSAJE
    // Formato esperado:
    // "Temp:XX.XXC, Hum:XX.XX% [ESTADO: NORMAL]"
    // "Temp:XX.XXC, Hum:XX.XX% [ALERTA: UMBRAL SUPERADO]"
    // ==========================================
    int idxTemp = mensaje.indexOf("Temp:");
    int idxC    = mensaje.indexOf("C,");
    int idxHum  = mensaje.indexOf("Hum:");
    int idxPct  = mensaje.indexOf("%");
    int idxEst  = mensaje.indexOf("[ESTADO:");
    int idxAlert = mensaje.indexOf("[ALERTA:");

    bool paqueteValido = (idxTemp != -1 && idxC != -1 &&
                          idxHum  != -1 && idxPct != -1 &&
                          (idxEst != -1 || idxAlert != -1));

    if (paqueteValido) {
      float temperatura = mensaje.substring(idxTemp + 5, idxC).toFloat();
      float humedad     = mensaje.substring(idxHum + 4, idxPct).toFloat();
      bool enAlerta     = (idxAlert != -1);

      Serial.print("  -> Temp: ");
      Serial.print(temperatura);
      Serial.print(" °C | Hum: ");
      Serial.print(humedad);
      Serial.print(" % | Estado: ");
      Serial.println(enAlerta ? "ALERTA" : "NORMAL");

      // Replicar la lógica del emisor: LED encendido si hay alerta
      if (enAlerta) {
        digitalWrite(PIN_LED, HIGH);
        Serial.println("  -> LED del receptor ENCENDIDO (alerta recibida).");
      } else {
        digitalWrite(PIN_LED, LOW);
        Serial.println("  -> LED del receptor APAGADO (estado normal).");
      }
    } else {
      Serial.println("  -> Paquete recibido pero con formato inesperado. Se ignora.");
    }
  }

  // ==========================================
  // TAREA 3: DETECCIÓN DE PÉRDIDA DE SEÑAL
  // ==========================================
  if (!senalPerdida && (millis() - tiempoUltimoPaquete > tiempoTimeout)) {
    senalPerdida = true;
    digitalWrite(PIN_LED, LOW);   // Apagar LED por seguridad
    Serial.println("¡ADVERTENCIA! No se reciben paquetes del emisor (>10 s).");
  }
}
