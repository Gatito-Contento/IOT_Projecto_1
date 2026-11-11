#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <HTTPClient.h>

// --- Pines LoRa y MOSFET ---
#define SS 5
#define RST 14
#define DIO0 2
#define PIN_MOSFET 22

// --- Credenciales WiFi ---
const char* ssid = "HONOR";
const char* wifiPassword = "Evett.san1412345";

// --- Configuración ThingSpeak (REST) ---
const String THINGSPEAK_API_KEY = "MUB87I425Y6TTSF1";
const String THINGSPEAK_URL = "http://api.thingspeak.com/update";

// --- Control de tasa de envío (ThingSpeak gratis: 1 update / 15 s) ---
unsigned long ultimoEnvio = 0;
const unsigned long INTERVALO_ENVIO = 15000; // 15 segundos

void setup() {
Serial.begin(115200);
while (!Serial);

pinMode(PIN_MOSFET, OUTPUT);
digitalWrite(PIN_MOSFET, LOW);

// Conectar a WiFi
Serial.print("Conectando a WiFi: ");
Serial.println(ssid);
WiFi.begin(ssid, wifiPassword);

unsigned long startAttempt = millis();
while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000) {
delay(500);
Serial.print(".");
}

if (WiFi.status() == WL_CONNECTED) {
Serial.println("\nWiFi conectado. IP: " + WiFi.localIP().toString());
} else {
Serial.println("\nError: No se pudo conectar a WiFi.");
}

// Configurar LoRa
Serial.println("Iniciando Receptor LoRa...");
LoRa.setPins(SS, RST, DIO0);

if (!LoRa.begin(915E6)) {
Serial.println("Error: No se encontró el módulo LoRa.");
while (1);
}
Serial.println("Esperando mensajes...");
}

void loop() {
int packetSize = LoRa.parsePacket();

if (packetSize) {
String dato = "";

while (LoRa.available()) {
dato += LoRa.readString();
}

int rssi = LoRa.packetRssi();

Serial.print("Mensaje recibido: '");
Serial.print(dato);
Serial.print("' RSSI: ");
Serial.println(rssi);

// --- Control del MOSFET ---
bool alerta = (dato.indexOf("[ALERTA: UMBRAL SUPERADO]") != -1);
if (alerta) {
digitalWrite(PIN_MOSFET, HIGH);
Serial.println(">> ALERTA: Pin 22 ACTIVADO (Motor encendido)");
} else if (dato.indexOf("[ESTADO: NORMAL]") != -1) {
digitalWrite(PIN_MOSFET, LOW);
Serial.println(">> NORMAL: Pin 22 APAGADO (Motor apagado)");
}

// --- Parsear temperatura y humedad ---
float temperatura = extraerValor(dato, "Temp:", "C");
float humedad = extraerValor(dato, "Hum:", "%");

Serial.print("Temp extraída: ");
Serial.println(temperatura, 2);
Serial.print("Hum extraída: ");
Serial.println(humedad, 2);

// --- Enviar a ThingSpeak (respetando 15 s) ---
if (millis() - ultimoEnvio >= INTERVALO_ENVIO) {
enviarAThingSpeak(temperatura, humedad, alerta ? 1 : 0);
ultimoEnvio = millis();
} else {
Serial.println("Esperando ventana de 15s para ThingSpeak...");
}
}
}

// Extrae un número flotante entre un prefijo y un sufijo
float extraerValor(String texto, String prefijo, String sufijo) {
int inicio = texto.indexOf(prefijo);
if (inicio == -1) return NAN;
inicio += prefijo.length();

int fin = texto.indexOf(sufijo, inicio);
if (fin == -1) return NAN;

String valorStr = texto.substring(inicio, fin);
valorStr.trim();
return valorStr.toFloat();
}

void enviarAThingSpeak(float temperatura, float humedad, int estadoMotor) {
if (WiFi.status() != WL_CONNECTED) {
Serial.println("WiFi desconectado. No se puede enviar a ThingSpeak.");
return;
}

HTTPClient http;

// field1 = temperatura, field2 = humedad, field3 = estado motor
String url = THINGSPEAK_URL + "?api_key=" + THINGSPEAK_API_KEY +
"&field1=" + String(temperatura, 2) +
"&field2=" + String(humedad, 2) +
"&field3=" + String(estadoMotor);

Serial.println("Enviando a ThingSpeak: " + url);

http.begin(url);
int httpCode = http.GET();

if (httpCode > 0) {
String payload = http.getString();
Serial.println("ThingSpeak OK. Entrada: " + payload);
} else {
Serial.println("Error en envío a ThingSpeak: " + String(httpCode));
}
http.end();
}
