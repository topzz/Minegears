#include <Arduino.h>
#include <mwc_stepper.h>



/* Global Variable for motor Controller */

// setting PWM properties
const int PWM_freq = 5000;
const int PWM1_Ch = 0;
const int PWM2_Ch = 2;
const int PWM_res = 12;
int duty_Cycle = 200;

// Variable for motor
#define RPWM1       32
#define LPWM1       33
#define motor_EN1   25

#define RPWM2       26
#define LPWM2       27
#define motor_EN2   14


/* Global Variable for Payload */

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


TaskHandle_t Task1;
TaskHandle_t Task2;

void setup()
{
    Serial.begin(115200);

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

    digitalWrite(EN1, HIGH);    // enable the pwm mode of the motor driver
    digitalWrite(EN2, HIGH);    // enable the pwm mode of the motor driver

    nema23.init();

    pinMode(MOTOR_pin1, OUTPUT);
    pinMode(MOTOR_pin2, OUTPUT);

    digitalWrite(MOTOR_pin1, LOW);
    digitalWrite(MOTOR_pin2, LOW);



    //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
    xTaskCreatePinnedToCore(
        received_command,   /* Task function. */
        "Task1",     /* name of task. */
        20000,       /* Stack size of task */
        NULL,        /* parameter of the task */
        1,           /* priority of the task */
        &Task1,      /* Task handle to keep track of created task */
        0);          /* pin task to core 0 */
    delay(500);
  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
    xTaskCreatePinnedToCore(
        exec_command,   /* Task function. */
        "Task2",     /* name of task. */
        20000,       /* Stack size of task */
        NULL,        /* parameter of the task */
        1,           /* priority of the task */
        &Task2,      /* Task handle to keep track of created task */
        1);          /* pin task to core 0 */
    delay(500);
}


void loop()
{    
}

/*
* Loop for the control of the motors
*/
received_command(void * pvParameters)
{
    for (;;) 
    {
        while(Serial.available()>0)
        {
            command= Serial.readStringUntil('\n');
            Serial.println(command);
        }

        if(command=="forward")
        {
            forward=true;
            command="";
        }
        if (command=="reverse")
        {
            reverse=true;
            command="";
        }
        if (command=="turn_left")
        {
            turn_left=true;
            command="";
        }
        if (command=="turn_right")
        {
            turn_right=true;
            command="";
        }
        if (command=="turn_360")
        {
            turn_360=true;
            command="";
        }
        if (command=="drill")
        {
            drill=true;
            command="";
        }
        if (command=="undrill")
        {
            undrill=true;
            command="";
        }
        if (command=="collect")
        {
            collect=true;
            command="";
        }
        if (command=="")
        {
            /* code */
        }
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
}

/*
* Loop for the control of the payload
*/
exec_command(void * pvParameters)
{
    for (;;) 
    {
        while(forward)
        {

        }
        while (reverse)
        {
            /* code */
        }
        
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
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


void m1_forward(int d_cycle)
{
  digitalWrite(LPWM1, HIGH);
  digitalWrite(RPWM1, LOW);
  ledcWrite(PWM1_Ch, d_cycle);
}
void m1_reverse(int d_cycle)
{
  digitalWrite(LPWM1, LOW);
  digitalWrite(RPWM1, HIGH);
  ledcWrite(PWM1_Ch, d_cycle);
}
void m2_forward(int d_cycle)
{
  digitalWrite(LPWM2, HIGH);
  digitalWrite(RPWM2, LOW);
  ledcWrite(PWM2_Ch, d_cycle);
}
void m2_reverse(int d_cycle)
{
  digitalWrite(LPWM2, LOW);
  digitalWrite(RPWM2, HIGH);
  ledcWrite(PWM2_Ch, d_cycle);
}

void motor_stop()
{
  digitalWrite(RPWM1, LOW);
  digitalWrite(LPWM1, LOW);
  digitalWrite(RPWM2, LOW);
  digitalWrite(LPWM2, LOW);
}
