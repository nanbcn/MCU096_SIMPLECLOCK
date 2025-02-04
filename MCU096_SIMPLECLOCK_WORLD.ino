#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <U8g2lib.h>
#include <NTPClient.h>
#include <ESP8266WebServer.h>

// Reemplaza con tus credenciales de red
const char* ssid = "XXXXXXXXXXXXXXXX";
const char* password = "XXXXXXXXXXXX";

// Configuración de la pantalla OLED
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 12, /* data=*/ 14);

// Configuración del cliente NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // Sin offset inicial, actualiza cada 60 segundos

// Configuración del servidor web
ESP8266WebServer server(80);

// Variables para la gestión de la zona horaria
int timeZoneOffset = 0; // Offset de la zona horaria en segundos
bool isDST = false; // Horario de verano

void setup() {
  Serial.begin(115200);
  Serial.println("Inicializando...");

  // Inicializar la pantalla OLED
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.sendBuffer();

  // Conectar a Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Conectado a Wi-Fi");

  // Inicializar el cliente NTP
  timeClient.begin();
  timeClient.update();

  // Configurar rutas del servidor web
  server.on("/", handleRoot);
  server.on("/setTimeZone", handleSetTimeZone);
  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  // Actualizar la hora desde el servidor NTP
  timeClient.update();

  // Procesar solicitudes HTTP
  server.handleClient();

  // Obtener la hora actual
  time_t rawTime = timeClient.getEpochTime();
  struct tm *tiempoActual = gmtime(&rawTime);
  
  // Ajustar la hora según la zona horaria y horario de verano
  tiempoActual->tm_sec += timeZoneOffset;
  mktime(tiempoActual);
  if (isDST) {
    tiempoActual->tm_hour += 1;
    if (tiempoActual->tm_hour >= 24) {
      tiempoActual->tm_hour -= 24;
      tiempoActual->tm_mday += 1;
    }
  }

  char formattedTime[9];
  sprintf(formattedTime, "%02d:%02d:%02d", tiempoActual->tm_hour, tiempoActual->tm_min, tiempoActual->tm_sec);

  Serial.printf("Hora actual: %s\n", formattedTime);

  // Mostrar la hora en la pantalla OLED
  displayTime(formattedTime);

  delay(1000); // Actualizar cada segundo
}

void handleRoot() {
  String html = "<html><body><h1>Configuracion de Zona Horaria</h1>"
                "<form action=\"/setTimeZone\" method=\"POST\">"
                "Offset (segundos): <input type=\"number\" name=\"offset\"><br>"
                "Horario de Verano: <input type=\"checkbox\" name=\"dst\"><br>"
                "<input type=\"submit\" value=\"Set Time Zone\">"
                "</form></body></html>";
  server.send(200, "text/html", html);
}

void handleSetTimeZone() {
  if (server.hasArg("offset")) {
    timeZoneOffset = server.arg("offset").toInt();
  }
  isDST = server.hasArg("dst");

  server.send(200, "text/html", "<html><body><h1>Zona Horaria Actualizada!</h1>"
                                "<a href=\"/\">Volver</a></body></html>");
}

void displayTime(char* time) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso28_tn); // Elegir una fuente adecuada
  u8g2.drawStr(0, 50, time);
  u8g2.sendBuffer();
}