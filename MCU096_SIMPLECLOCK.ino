#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <U8g2lib.h>
#include <NTPClient.h>

// Reemplaza con tus credenciales de red
const char* ssid = "XXXXXXXXXXXXXXXX";
const char* password = "XXXXXXXXXXXX";

// Configuración de la pantalla OLED
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 12, /* data=*/ 14);

// Configuración del cliente NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // Sin offset inicial, actualiza cada 60 segundos

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
}

void loop() {
  // Actualizar la hora desde el servidor NTP
  timeClient.update();

  // Obtener la hora actual
  time_t rawTime = timeClient.getEpochTime();
  struct tm *tiempoActual = gmtime(&rawTime);
  
  // Ajustar la hora para España (CET/CEST)
  tiempoActual->tm_hour += isDaylightSavingTime(tiempoActual) ? 2 : 1;

  // Ajustar si hemos cruzado el límite del día
  if (tiempoActual->tm_hour >= 24) {
    tiempoActual->tm_hour -= 24;
    tiempoActual->tm_mday += 1;
  }

  char formattedTime[9];
  sprintf(formattedTime, "%02d:%02d:%02d", tiempoActual->tm_hour, tiempoActual->tm_min, tiempoActual->tm_sec);
  
  Serial.printf("Hora actual: %s\n", formattedTime);

  // Mostrar la hora en la pantalla OLED
  displayTime(formattedTime);

  delay(1000); // Actualizar cada segundo
}

bool isDaylightSavingTime(struct tm *timeinfo) {
  // Horario de verano empieza el último domingo de marzo a las 2:00 y termina el último domingo de octubre a las 3:00
  if (timeinfo->tm_mon < 2 || timeinfo->tm_mon > 9) return false; // Enero, Febrero, Noviembre, Diciembre
  if (timeinfo->tm_mon > 2 && timeinfo->tm_mon < 9) return true;  // Abril a Septiembre

  int lastSunday = timeinfo->tm_mday - timeinfo->tm_wday; // Último domingo del mes

  if (timeinfo->tm_mon == 2) // Marzo
    return lastSunday >= 25 && (timeinfo->tm_hour >= 2 || (timeinfo->tm_hour == 1 && timeinfo->tm_min == 59));

  if (timeinfo->tm_mon == 9) // Octubre
    return lastSunday < 25 || (lastSunday == 25 && timeinfo->tm_hour < 3);

  return false;
}

void displayTime(char* time) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso28_tn); // Elegir una fuente adecuada
  u8g2.drawStr(0, 50, time);
  u8g2.sendBuffer();
}