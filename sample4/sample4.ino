/**
   ----------------------------------------------------------------------------
   ESP32 Remote Control with WebSocket
   ----------------------------------------------------------------------------
   © 2020 Stéphane Calderoni
   ----------------------------------------------------------------------------
*/

#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// ----------------------------------------------------------------------------
// Definition of macros
// ----------------------------------------------------------------------------
#define LED_PIN 13
#define HTTP_PORT 80

// WiFi credentials
//const char *WIFI_SSID = "SuMMO Center";
//const char *WIFI_PASS = "summoindustry4.0";
//const char *WIFI_SSID = "Free WiFi";
//const char *WIFI_PASS = "N0p4ssw0rd";
const char *WIFI_SSID = "CRV";
const char *WIFI_PASS = "12345678900987654321";

// ----------------------------------------------------------------------------
// Definition of global variables
// ----------------------------------------------------------------------------

AsyncWebServer server(HTTP_PORT);
AsyncWebSocket ws("/ws");

// ----------------------------------------------------------------------------
// Definition of global constants
// ----------------------------------------------------------------------------

struct Led
{
  // state variables
  uint8_t pin;
  bool on;

  // methods
  void update()
  {
    digitalWrite(pin, on ? HIGH : LOW);
  }
};

Led onboard_led = {26, false};
Led led = {LED_PIN, false};

boolean driving = false;

int _speed;
int _angle;
int map_speed;

// ----------------------------------------------------------------------------
// SPIFFS initialization
// ----------------------------------------------------------------------------

void initSPIFFS()
{
  if (!SPIFFS.begin())
  {
    Serial.println("Cannot mount SPIFFS volume...");
    while (1)
    {
      onboard_led.on = millis() % 200 < 50;
      onboard_led.update();
    }
  }
}
// ----------------------------------------------------------------------------
// WiFi initialization
// ----------------------------------------------------------------------------

void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.printf("Trying to connect [%s] ", WiFi.macAddress().c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.printf(" %s\n", WiFi.localIP().toString().c_str());
}
// ----------------------------------------------------------------------------
// Web server initialization
// ----------------------------------------------------------------------------

void initWebServer()
{
  server.on("/", onRootRequest);
  server.serveStatic("/", SPIFFS, "/");
  server.begin();
}
// ----------------------------------------------------------------------------
// WebSocket initialization
// ----------------------------------------------------------------------------

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}
// ----------------------------------------------------------------------------
// Handle message from websocket
// ----------------------------------------------------------------------------
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {

    const uint8_t size = JSON_OBJECT_SIZE(200);
    StaticJsonDocument<size> json;
    DeserializationError err = deserializeJson(json, data);
    if (err)
    {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.c_str());
      return;
    }
    int x_coor = json["x"];
    int y_coor = json["y"];
    _speed = json["speed"];
    _angle = json["angle"];

    //    map_speed = map(_speed, 0, 100, 0, 4095 );
    //    Serial.print("Map Speed= "); Serial.println(map_speed);
    if (_speed != 0)
    {
      driving = true;
    }
    else {
      driving = false;
    }
  }
}

void onEvent(AsyncWebSocket *server,
             AsyncWebSocketClient *client,
             AwsEventType type,
             void *arg,
             uint8_t *data,
             size_t len)
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

void notifyClients()
{
  const uint8_t size = JSON_OBJECT_SIZE(1);
  StaticJsonDocument<size> json;
  json["status"] = led.on ? "on" : "off";

  char data[17];
  size_t len = serializeJson(json, data);
  ws.textAll(data, len);
}

String processor(const String &var)
{
  return String(var == "STATE" && led.on ? "on" : "off");
}

void onRootRequest(AsyncWebServerRequest *request)
{
  request->send(SPIFFS, "/index.html", "text/html", false, processor);
}


// setting PWM properties
const int PWM_freq = 5000;
const int PWM1_Ch = 0;
const int PWM2_Ch = 2;
const int PWM_res = 12;
int duty_Cycle = 200;
// ----------------------------------------------------------------------------
// Variable for motor
// ----------------------------------------------------------------------------
int RPWM1 = 32;
int LPWM1 = 33;
int EN1 = 25;

int RPWM2 = 26;
int LPWM2 = 27;
int EN2 = 14;

// ----------------------------------------------------------------------------
// Setup
// ----------------------------------------------------------------------------

void setup()
{
  pinMode(13, OUTPUT);
  pinMode(RPWM1, OUTPUT);
  pinMode(LPWM1, OUTPUT);
  pinMode(EN1, OUTPUT);

  ledcSetup(PWM1_Ch, PWM_freq, PWM_res);
  ledcAttachPin(EN1, PWM1_Ch);

  pinMode(RPWM2, OUTPUT);
  pinMode(LPWM2, OUTPUT);
  pinMode(EN2, OUTPUT);

  ledcSetup(PWM2_Ch, PWM_freq, PWM_res);
  ledcAttachPin(EN2, PWM2_Ch);

  motor_stop();



  pinMode(onboard_led.pin, OUTPUT);
  pinMode(led.pin, OUTPUT);

  Serial.begin(115200);
  delay(500);

  initSPIFFS();
  initWiFi();
  initWebSocket();
  initWebServer();

digitalWrite(13,HIGH);
  digitalWrite(EN1, HIGH);
  digitalWrite(EN2, HIGH);
}

// ----------------------------------------------------------------------------
// Main control loop
// ----------------------------------------------------------------------------

void loop()
{
  ws.cleanupClients();
  while (driving)
  {
    motor_controller(_angle, _speed);
  }
  while (driving == false)
  {
    motor_stop();
  }
}

//void motor_controller(int angle, int speed)
//{
//  if (angle > 0 && angle < 180 && speed != 0)
//  {
//    forward();
//  }
//  if (angle > 180 && angle < 360 && speed != 0)
//  {
//    backward();
//  }
//}

//Left motor= PWM1    Right motor= PWM2

//void forward()
//{
//  //motor left and right forward
//  digitalWrite(LPWM1, HIGH);
//  digitalWrite(RPWM1, LOW);
//
//  digitalWrite(LPWM2, HIGH);
//  digitalWrite(RPWM2, LOW);
//
//  Serial.println("Forward");
//}
//void backward()
//{
//  //motor left and right backward
//  digitalWrite(RPWM1, HIGH);
//  digitalWrite(LPWM1, LOW);
//
//  digitalWrite(RPWM2, HIGH);
//  digitalWrite(LPWM2, LOW);
//
//  Serial.println("reverse");
//}
//void turnLeft()
//{
//  //left motor backward
//  digitalWrite(RPWM1, HIGH);
//  digitalWrite(LPWM1, LOW);
//  //right motor forward
//
//}
//void turnRight()
//{
//  //motor left forward
//  digitalWrite(LPWM1, HIGH);
//  digitalWrite(RPWM1, LOW);
//  //motor right backward
//
//}
//void RevLeft()
//{
//
//}
//void RevRight()
//{
//
//}

void motor_controller(int angle, int speed)
{
  if (angle > 0 && angle <= 30 && speed != 0 )
  {
    //    motor1&2=FW
    //    motor3&4=BW
    m1_forward(4095);
    m2_reverse(4095);
    Serial.println("turn CW");
  }
  if (angle > 30 && angle <= 60 && speed != 0)
  {
    //    motor1&2=FW
    //    motor3&4=FWH
    m1_forward(4095);
    m2_forward(0);
    Serial.println("turn right");
  }
  if (angle > 60 && angle <= 120 && speed != 0)
  {
    //    motor1,2,=FW
    //    motor3,4=FWT
    m1_forward(4095);
    m2_forward(4095);
    Serial.println("mild turn right");
  }
  //  if (angle > 90 && angle <= 120 && speed != 0)
  //  {
  //    //    motor3,4=FW
  //    //    motor1,2=FWT
  //    m2_forward(4095);
  //    m1_forward(3070);
  //    Serial.println("mild turn left");
  //  }
  if (angle > 120 && angle <= 150 && speed != 0)
  {
    //    motor3,4=FW
    //    motor1,2=FWH
    m2_forward(4095);
    m1_forward(0);
    Serial.println("turn left");
  }
  if (angle > 150 && angle <= 180 && speed != 0)
  {
    //    motor3,4=FW
    //    motor1,2=BW
    m2_forward(4095);
    m1_reverse(4095);
    Serial.println("turn CCW");
  }
  if (angle > 180 && angle <= 210 && speed != 0)
  {
    //    motor3,4=BW
    //    motor1,2=FWH
    m2_reverse(4095);
    m1_reverse(0);
    Serial.println("hard reverse to left");
  }
  if (angle > 210 && angle <= 240 && speed != 0)
  {
    //    motor3,4=BW
    //    motor1,2=BWH
    m2_reverse(4095);
    m1_reverse(2048);
    Serial.println("reverse to left");
  }
  if (angle > 240 && angle <= 300 && speed != 0)
  {
    //    motor3,4=BW
    //    motor1,2=BWT
    m2_reverse(4095);
    m1_reverse(4095);
    Serial.println("mild reverse to left");
  }
  //  if (angle > 270 && angle <= 300 && speed != 0)
  //  {
  //    //    motor1,2=BW
  //    //    motor3,4=BWT
  //    m1_reverse(4095);
  //    m2_reverse(3070);
  //    Serial.println("mild reverse to right");
  //  }
  if (angle > 300 && angle <= 330 && speed != 0)
  {
    //    motor1,2=BW
    //    motor3,4=BWH
    m1_reverse(4095);
    m2_reverse(2048);
    Serial.println("reverse to right");
  }
  if (angle > 330 && angle <= 360 && speed != 0)
  {
    //    motor1,2=BW
    //    motor3,4=FW
    m1_reverse(4095);
    m2_reverse(0);
    Serial.println("hard reverse to right");
  }
  if (angle == 0 && speed == 0)
  {
    Serial.println("motor stop");
  }
}


void m1_forward(int d_cycle)
{
  digitalWrite(LPWM1, HIGH);
  digitalWrite(RPWM1, LOW);
  ledcWrite(PWM1_Ch, d_cycle);
  //  for (int i = 0; i < d_cycle ; i + 512)
  //  {
  //    ledcWrite(PWM1_Ch, i);
  //    delay(10);
  //  }


}
void m1_reverse(int d_cycle)
{
  digitalWrite(LPWM1, LOW);
  digitalWrite(RPWM1, HIGH);
  ledcWrite(PWM1_Ch, d_cycle);
  //  for (int i = 0; i < d_cycle ; i + 512)
  //  {
  //    ledcWrite(PWM1_Ch, i);
  //    delay(10);
  //  }
}
void m2_forward(int d_cycle)
{
  digitalWrite(LPWM2, HIGH);
  digitalWrite(RPWM2, LOW);
  ledcWrite(PWM2_Ch, d_cycle);
  //  for (int i = 0; i < d_cycle ; i + 512)
  //  {
  //    ledcWrite(PWM2_Ch, i);
  //    delay(10);
  //  }
}
void m2_reverse(int d_cycle)
{
  digitalWrite(LPWM2, LOW);
  digitalWrite(RPWM2, HIGH);
  ledcWrite(PWM2_Ch, d_cycle);
  //  for (int i = 0; i < d_cycle ; i + 512)
  //  {
  //    ledcWrite(PWM1_Ch, i);
  //    delay(10);
  //  }
}

void motor_stop()
{
  digitalWrite(RPWM1, LOW);
  digitalWrite(LPWM1, LOW);
  digitalWrite(RPWM2, LOW);
  digitalWrite(LPWM2, LOW);
  //  for (int i = d_cycle; i > 0 ; i--)
  //  {
  //    ledcWrite(PWM1_Ch, i);
  //    delay(10);
  //    ledcWrite(PWM2_Ch, i);
  //    delay(10);
  //  }

}
