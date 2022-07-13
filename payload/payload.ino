/*
  For Websocket
*/
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <ArduinoJson.h>


bool turn = false;
bool drill = false;
bool retract = false;
bool automate = false;
/*
  For motor
*/
#include <mwc_stepper.h>

#define EN_PIN 25
#define DIR_PIN 26
#define STEP_PIN 27

#define MOTOR_pin1 18
#define MOTOR_pin2 19

#define RPM 60

#define PULSE 400

#define ClOCKWISE 1
#define OTHERWISE 0

MWCSTEPPER nema23(EN_PIN, DIR_PIN, STEP_PIN);

const byte numChars = 32;
char receivedChars[numChars]; // an array to store the received data

boolean newData = false;
String a = "";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");



// Set your Static IP address
IPAddress local_IP(192, 168, 4,1);
// Set your Gateway IP address
IPAddress gateway(192, 168, 100, 1);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

// WiFi credentials
const char *ssid = "SuMMO Center";
const char *password = "summoindustry4.0";
//const char *ssid = "CRV";
//const char *password = "12345678900987654321";

void notifyClients()
{
  const uint8_t size = JSON_OBJECT_SIZE(1);
  StaticJsonDocument<size> json;
  json["status"] = "content";

  char data[20];
  size_t len = serializeJson(json, data);

  ws.textAll(data, len);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {

    data[len] = 0;
    if (strcmp((char *)data, "drill") == 0)
    {
      Serial.println("drill");
      drill = true;
      notifyClients();
    }
    if (strcmp((char *)data, "retract") == 0)
    {
      Serial.println("retract");
      retract = true;
      notifyClients();
    }
    if (strcmp((char *)data, "turn") == 0)
    {
      Serial.println("turn");
      turn = true;
      notifyClients();
    }
    if (strcmp((char *)data, "auto") == 0)
    {
      Serial.println("auto");
      automate = true;
      notifyClients();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}
void initSPIFFS()
{
  if (!SPIFFS.begin())
  {
    Serial.println("Cannot mount SPIFFS volume...");
  }
}
void initWiFi()
{
//  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
//    Serial.println("STA Failed to configure");
//  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("Trying to connect [%s] ", WiFi.macAddress().c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.printf(" %s\n", WiFi.localIP().toString().c_str());

}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void initWebServer()
{
  
  server.on("/", onRootRequest);
  server.serveStatic("/", SPIFFS, "/");
  server.begin();
}

String processor(const String &var)
{
  Serial.println(var);
  if (var == "STATE")
  {
    Serial.println("OK processor");
  }
}
void onRootRequest(AsyncWebServerRequest *request)
{
  request->send(SPIFFS, "/index.html", "text/html", false, processor);
}

/*
  Setup run once
*/
void setup()
{
  Serial.begin(115200);
  nema23.init();

  pinMode(MOTOR_pin1, OUTPUT);
  pinMode(MOTOR_pin2, OUTPUT);
  digitalWrite(MOTOR_pin1, LOW);
  digitalWrite(MOTOR_pin2, LOW);
  // nema23.active(DEACTIVE);

  initSPIFFS();
  initWiFi();
  initWebSocket();
  initWebServer();
}

void loop()
{
  ws.cleanupClients();

  while (turn)
  {
    _turn();
    turn = false;
  }
  while (drill)
  {
    _drill();
    drill = false;
  }
  while (retract)
  {
    _retract();
    retract = false;
  }
  while (automate)
  {
    _drill();
    delay(1000);
    _retract();
    delay(1000);
    _turn();
    automate = false;
  }
}

void _turn()
{
  nema23.set(ClOCKWISE, RPM, PULSE);

  for (size_t i = 0; i < 37; i++)
  {
    nema23.run();
  }
}
void _drill()
{
  for (int i = 0; i < 20; i++)
  {
    digitalWrite(MOTOR_pin1, HIGH);
    digitalWrite(MOTOR_pin2, LOW);
    delay(500);
  }
  digitalWrite(MOTOR_pin1, LOW);
  digitalWrite(MOTOR_pin2, LOW);
}
void _retract()
{
  for (int i = 0; i < 20; i++)
  {
    digitalWrite(MOTOR_pin2, HIGH);
    digitalWrite(MOTOR_pin1, LOW);
    delay(500);
  }
  digitalWrite(MOTOR_pin1, LOW);
  digitalWrite(MOTOR_pin2, LOW);
}
