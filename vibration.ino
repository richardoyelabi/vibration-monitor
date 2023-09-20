#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

const size_t capacity = JSON_OBJECT_SIZE(3);
DynamicJsonDocument doc(1024);

/* Put your SSID & Password */
const char *ssid = "vibration";
const char *password = "12341234";

ESP8266WebServer server(80);

#define PI 3.14159265

LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_MPU6050 mpu;

double vib, mpuTemp, freqMPU;               // vibration params

unsigned long lastTime = 0;
unsigned long timerDelay = 500;

double round2(double val){
  return (int)(val * 100 + 0.5)/100.0;
}

void sendData()
{
  // vibration params
  doc["vib"] = vib;
  doc["temp"] = mpuTemp;
  String json;
  serializeJson(doc, json);
  Serial.println("json" + json);
  server.send(200, "text/plain", json);
}

void setup() {
  Serial.begin(9600);
  mpu.begin();
  lcd.init();
  lcd.backlight();
  SPIFFS.begin();

  // SERVER INIT
  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(ssid, password) ? "Ready" : "Failed!");

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
  delay(100);

  // Which routine to handle at root location. This is display page
  server.on("/", handleRoot);
  server.on("/getData", sendData);

  // called when the url is not defined here
  // use it to load content from SPIFFS
  server.onNotFound([]()
  {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });

  server.begin();
  Serial.println("HTTP server started");

}

void loop() {
  server.handleClient();

  //  lcd.clear();
  if (millis() >= lastTime + timerDelay)
  {

    /* Get new sensor events with the readings */
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    /* Print out the values */
    Serial.print("Acceleration X: ");
    Serial.print(a.acceleration.x);
    Serial.print(", Y: ");
    Serial.print(a.acceleration.y);
    Serial.print(", Z: ");
    Serial.print(a.acceleration.z);
    Serial.println(" m/s^2");

    // Read the raw accelerometer values
    int16_t ax, ay, az;
    ax = 16384 * a.acceleration.x;
    ay = 16384 * a.acceleration.y;
    az = 16384 * a.acceleration.z;

    // Calculate the vibration frequency and amplitude
    freqMPU = round2(a.acceleration.x / (2 * PI));

    // Print the vibration frequency and amplitude
    Serial.print("Frequency: ");
    Serial.print(freqMPU);
    Serial.print(" Hz  ");

    vib = round2(sqrt(ax * ax + ay * ay + az * az) / 16384.0);

    Serial.print("Vibration: ");
    Serial.println(vib);

    mpuTemp = round2(temp.temperature);
    Serial.print("Temperature: ");
    Serial.print(mpuTemp);
    Serial.println(" degC");

    Serial.println("");

    lastTime = millis();
  }

  lcd.setCursor(0, 0);
  lcd.print("Vibration:");
  lcd.println(String(vib)+"          ");

  lcd.setCursor(0, 1);
  lcd.print("Temp:");
  lcd.println(String(mpuTemp) + "C              ");
}

void handleRoot()
{
  server.sendHeader("Location", "/index.html", true); // Redirect to our html web page
  server.send(302, "text/plane", "");
}

bool handleFileRead(String path)
{
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/"))
    path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
  {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

String getContentType(String filename)
{
  if (server.hasArg("download"))
    return "application/octet-stream";
  else if (filename.endsWith(".htm"))
    return "text/html";
  else if (filename.endsWith(".html"))
    return "text/html";
  else if (filename.endsWith(".css"))
    return "text/css";
  else if (filename.endsWith(".js"))
    return "application/javascript";
  else if (filename.endsWith(".png"))
    return "image/png";
  else if (filename.endsWith(".gif"))
    return "image/gif";
  else if (filename.endsWith(".jpg"))
    return "image/jpeg";
  else if (filename.endsWith(".ico"))
    return "image/x-icon";
  else if (filename.endsWith(".xml"))
    return "text/xml";
  else if (filename.endsWith(".pdf"))
    return "application/x-pdf";
  else if (filename.endsWith(".zip"))
    return "application/x-zip";
  else if (filename.endsWith(".gz"))
    return "application/x-gzip";
  return "text/plain";
}
