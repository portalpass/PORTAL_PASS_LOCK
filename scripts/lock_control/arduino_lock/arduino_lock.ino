#include <Servo.h>  //servo library

//PINS
#define LED                   12    //general LED controlled by APP
//      LED_BUILTIN           13    //door status LED (no need to define)
#define DOOR_STATUS            5    //magnetic door switch
#define LOCK_BUTTON            8    //interior toggled lock/unlock button
#define VIDEO_REQUEST         11    //request video button
#define SERVO_CONTROL          3    //servo control signal (PWM pin)
#define SERVO_POWER            2    //servo power (amplified by transistor)

//CONSTANTS
#define LOCKING_ANGLE         180   //locked position angle
#define UNLOCKING_ANGLE        90   //unlocked position angle
#define SERVO_DELAY           600   //delay for servo to move to position (milliseconds)
#define DEFAULT_TIMEOUT  (10*1000)  //default lock timeout
#define MIN_TIMEOUT      (10*1000)
#define MAX_TIMEOUT    0xFFFFFFFF   //max value of a 32 bit number

//serial_output function arguments (determine serial output message to be sent)
#define LED_REPLY               0   // "REPLY:STALE:"(0|1)...(off|on)
#define DS_REPLY                1   // "REPLY:STATD:"(0|1)...(closed|open)
#define LS_REPLY                2   // "REPLY:STATL:"(0|1)...(locked|unlocked)
#define LS_SPONT                3   // "SPONT:STATL:"(0|1)...(locked|unlocked)
#define ERROR_REPLY             4   // "REPLY:ERRDRO"        Door in open
#define VIDREQ_SPONT            5   // "SPONT:VIDREQ"
#define LOCKOUT_MS_REPLY        6   // ":"<milliseconds>

//special characters
#define CLEAR_CHAR              '*' //clears string in_str when received
#define TERMINATING_CHAR        'X' //terminates every command stored in string in_str

//serial input messages (commands received serially by Arduino)
const String UNLOCK_CMD     = "unl" + String(TERMINATING_CHAR);   //unlock
const String LOCK_CMD       = "loc" + String(TERMINATING_CHAR);   //lock
const String DOOR_STATE_Q   = "?ds" + String(TERMINATING_CHAR);   //check door status
const String LOCK_STATE_Q   = "?ls" + String(TERMINATING_CHAR);   //check lock status
const String LED_ON         = "hl1" + String(TERMINATING_CHAR);   //turn LED on
const String LED_OFF        = "ll1" + String(TERMINATING_CHAR);   //turn LED off
const String LED_Q          = "?l1" + String(TERMINATING_CHAR);   //check LED status
const String SET_TIMEOUT_MS =  "sT"; //set Timeout to what is between this header and TERMINATING_CHAR
                                    //if it is an invalid number, set to DEFAULT_TIMEOUT
const String TIMEOUT_Q      ="?lT"  + String(TERMINATING_CHAR); //reply with lock timeout in MSG

//headers for serial output messages
const String SPONT_HEADER = "SPONT:";   //signifies start of spontaneous message
const String REPLY_HEADER = "REPLY:";   //signifies start of reply message

//serial output messages (messages sent serially by Arduino)
const String LED_STATUS_MESSAGE     = "STALE:"; // (0|1)...(off|on)
const String DOOR_STATUS_MESSAGE    = "STATD:"; // (0|1)...(closed|open)
const String LOCK_STATUS_MESSAGE    = "STATL:"; // (0|1)...(locked|unlocked)
const String LOCK_TIMEOUT_MS_MESSAGE= "AUTTO:"; // locktimeout (in ms)
const String VIDEO_REQUEST_MESSAGE  = "VIDREQ";
const String ERR_DOOR_OPEN_MESSAGE  = "ERRDRO";

//VARIABLES
Servo lock_servo; //servo object

//string variables to store serial messages
String in_str;  //stores input from serial
String out_str; //stores output to serial

//state variables
int lock_state;             //1 => door unlocked,               0 => door locked
int door_state;             //1 => door open,                   0 => door closed (switch used is NO (normally opened))
int last_door_state;        //last state of door
int door_led_state;         //1 => door LED on (door closed),   0 => door LED off (door open)
int led_state;              //1 => general LED on,              0 => general LED off

//timing variables
unsigned long int time_of_event;    //stores time of last event (locking/unlocking/door state change)
unsigned long int lockout_time;
//video request button variables used for debouncing
int vid_request;
int old_vid_request;
long int vid_request_timeout_ms;
long int time_vid_request;

//interior toggled lock/unlock button variables used for debouncing
int toggle_lock;
int old_toggle_lock;
long int toggle_lock_timeout_ms;
long int time_toggle_lock;

void setup()
{
    //set pin modes
    pinMode(LED,            OUTPUT);            //general LED controlled by APP
    pinMode(LED_BUILTIN,    OUTPUT);            //door status LED (no need to define)
    pinMode(DOOR_STATUS,    INPUT_PULLUP);      //magnetic door switch (pulled to 1 by default)
    pinMode(LOCK_BUTTON,    INPUT);             //interior toggled lock/unlock button
    pinMode(VIDEO_REQUEST,  INPUT);             //request video button
    pinMode(SERVO_CONTROL,  OUTPUT);            //servo control signal
    pinMode(SERVO_POWER,    OUTPUT);            //servo power (amplified by transistor)

    //initialize output pins
    digitalWrite(LED,           0);     //set general LED controlled by APP to off
    digitalWrite(SERVO_POWER,   0);     //set servo power (amplified by transistor) to off

    //string variables to store serial messages
    String in_str = "";  //stores input from serial
    String out_str = ""; //stores output to serial

    //state variables
    lock_state = 1;             //1 => door unlocked (assumed)
    last_door_state = 1;        //last state of door (assume 1 => door open)
    led_state = 0;              //0 => general LED off

    //timing variables
    lockout_time  = DEFAULT_TIMEOUT;
    time_of_event = millis();   //assumes time of last event is at startup

    //video request button variables used for debouncing
    vid_request = 0;
    old_vid_request = 0;
    vid_request_timeout_ms = millis();
    time_vid_request = -1 * vid_request_timeout_ms;

    //interior toggled lock/unlock button variables used for debouncing
    toggle_lock = 0;
    old_toggle_lock = 0;
    toggle_lock_timeout_ms = millis();
    time_toggle_lock = -1 * toggle_lock_timeout_ms;

    //initialize serial communication
    Serial.begin(9600);
    Serial.flush();
}

void loop()
{
    check_door_status();

    //check for serial message
    char char_in;   //stores current character being read serially
    if(Serial.available())
    {
        char_in = (char) Serial.read();
        if ( (char_in == CLEAR_CHAR) || (char_in == '\n') ) //clear in_str if clear command received
        {
            in_str = "";
        }
        else    //append character received to in_str if clear command not received
        {
            in_str += char_in;
        }
    }

    //execute command once TERMINATING_CHAR received
    if (char_in == TERMINATING_CHAR)
    {
        if (in_str == UNLOCK_CMD)
        {
            if (unlock())
            {
                serial_output(LS_REPLY, 1);     // => successful unlocking
            }
            else
            {
                serial_output(ERROR_REPLY, 1);  // => unsuccessful unlocking
            }
        }
        else if (in_str == LOCK_CMD)
        {
            if (lock())
            {
                serial_output(LS_REPLY, 1);     // => successful locking
            }
            else
            {
                serial_output(ERROR_REPLY, 1);  // => unsuccessful locking
            }
        }
        else if (in_str == DOOR_STATE_Q)
        {
            serial_output(DS_REPLY, 1);
        }
        else if (in_str == LOCK_STATE_Q)
        {
            serial_output(LS_REPLY, 1);
        }
        else if (in_str == LED_ON)
        {
            led_state = 1;
            digitalWrite(LED, led_state);
            serial_output(LED_REPLY, 1);
        }
        else if (in_str == LED_OFF)
        {
            led_state = 0;
            digitalWrite(LED, led_state);
            serial_output(LED_REPLY, 1);
        }
        else if (in_str == LED_Q)
        {
            serial_output(LED_REPLY, 1);
        }
        else if ( in_str.startsWith(SET_TIMEOUT_MS) && in_str.endsWith( String (TERMINATING_CHAR ) ) )
        {
            //strip off SET_TIMEOUT HEADER and the terminating CHAR
            in_str = in_str.substring( SET_TIMEOUT_MS.length() );
            in_str = in_str.substring( 0, in_str.length() - 1 );
            lockout_time = constrain( in_str.toInt(), 0, MAX_TIMEOUT );
            if ( lockout_time <= MIN_TIMEOUT )
            {
                lockout_time = MIN_TIMEOUT;
            }
            time_of_event == millis();
            serial_output(LOCKOUT_MS_REPLY, 1);      
        }
        else if ( in_str == LED_Q )
        {
            serial_output(LOCKOUT_MS_REPLY, 1);      
        }
        in_str = "";    //clear in_str
    }

    //check if there is a video call request:
    vid_request = digitalRead(VIDEO_REQUEST);
    if ( vid_request == 1 && old_vid_request == 0 && millis() > time_vid_request + vid_request_timeout_ms )
    {
        time_vid_request = millis();
        serial_output(VIDREQ_SPONT, 2);
    }
    old_vid_request = vid_request;

    //check if LOCK_BUTTON pressed
    toggle_lock = digitalRead(LOCK_BUTTON);
    if ( toggle_lock == 1 && old_toggle_lock == 0 && millis() > time_toggle_lock + toggle_lock_timeout_ms )
    {
        time_toggle_lock = millis();
        if (lock_state) //true => unlocked, false => locked
        {
            if (lock())
            {
                serial_output(LS_SPONT, 2); //send spontaneous message only if successful locking
            }
        }
        else
        {
            if (unlock())
            {
                serial_output(LS_SPONT, 2); //send spontaneous message only if successful unlocking
            }
        }
    }
    old_toggle_lock = toggle_lock;

    //auto-lockout after set time:
    if ( millis() >= time_of_event + lockout_time )
    {
        if (lock())
        {
            serial_output(LS_SPONT, 2); // => successful locking
        }
        time_of_event = millis();
    }
}

//function to unlock door
//returns integer: 0 => unsuccessful unlocking, 1 => successful unlocking
int unlock()
{
    unsigned long long time_of_unlocking;                            //stores time of unlocking
    int success = 0;                                            //1 => successful unlocking, 0 => unsuccessful unlocking
    if (!check_door_status())                                   //check_door_status() returns: 0 => door closed, 1 => door opened
    {
        digitalWrite(SERVO_POWER, 1);                           //provide power to servo
        lock_servo.attach(SERVO_CONTROL);                       //attach servo to servo object when in use
        lock_servo.write(UNLOCKING_ANGLE);                      //move servo to unlocked position
        time_of_unlocking = millis();                           //stores time of unlocking
        while ((millis() - time_of_unlocking) < SERVO_DELAY) {} //wait for servo to move into position (do nothing)
        lock_state = 1;                                         //lock now assumed to be in unlocked position
        time_of_event = millis();                               //store time of unlocking
        digitalWrite(SERVO_POWER, 0);                           //cut power from servo when not in use
        lock_servo.detach();                                    //detach servo from servo object when not in use
        success = 1;                                            //assume successful unlocking
    }
    else                                                        //servo will not move to lock when door is opened
    {
        success = 0;                                            //assume unsuccessful unlocking (because door opened)
    }
    return success;                                             //return: 1 => successful unlocking, 0 => unsuccessful unlocking
}

//function to lock door
//returns integer: 0 => unsuccessful locking, 1 => successful locking
int lock()
{
    unsigned long long time_of_locking;                              //stores time of locking
    int success = 0;                                            //1 => successful locking, 0 => unsuccessful locking
    if (!check_door_status())                                   //check_door_status() returns: 0 => door closed, 1 => door opened
    {
        digitalWrite(SERVO_POWER, 1);                           //provide power to servo
        lock_servo.attach(SERVO_CONTROL);                       //attach servo to servo object when in use
        lock_servo.write(LOCKING_ANGLE);                        //move servo to locked position
        time_of_locking = millis();                             //stores time of locking
        while ((millis() - time_of_locking) < SERVO_DELAY)   {} //wait for servo to move into position (do nothing)
        lock_state = 0;                                         //lock now assumed to be in locked position
        time_of_event = millis();                               //store time of locking
        digitalWrite(SERVO_POWER, 0);                           //cut power from servo when not in use
        lock_servo.detach();                                    //detach servo from servo object when not in use
        success = 1;                                            //assume successful locking
    }
    else                                                        //servo will not move to unlock when door is opened
    {
        success = 0;                                            //assume unsuccessful locking (because door opened)
    }
    return success;                                             //return: 1 => successful locking, 0 => unsuccessful locking
}

//function to check if door is opened or closed
//returns integer: 0 => door closed, 1 => door opened
int check_door_status()
{
    if (!digitalRead(DOOR_STATUS))
    {
        door_state = 0; //door closed
        door_led_state = 1;
    }
    else
    {
        door_state = 1; //door opened
        door_led_state = 0;
    }
    digitalWrite(LED_BUILTIN, door_led_state);

    if (door_state != last_door_state)  //check if state of door has changed
    {
        time_of_event = millis();       //store time of door state change
    }
    last_door_state = door_state;

    return door_state;
}

//function to handle serial output of messages
void serial_output(int message, int type_of_message) //type_of_message: 1 => REPLY, 2 => SPONT
{
    if (type_of_message == 1)
    {
        if (message == DS_REPLY)
        {
            out_str = DOOR_STATUS_MESSAGE + String(door_state);
        }
        else if (message == LS_REPLY)
        {
            out_str = LOCK_STATUS_MESSAGE + String(lock_state);
        }
        else if (message == ERROR_REPLY)
        {
            out_str = ERR_DOOR_OPEN_MESSAGE;
        }
        else if (message == LED_REPLY)
        {
            out_str = LED_STATUS_MESSAGE + String(led_state);
        }
        else if (message == LOCKOUT_MS_REPLY )
        {
            out_str = LOCK_TIMEOUT_MS_MESSAGE + String( lockout_time );
        }
        out_str = REPLY_HEADER + out_str + TERMINATING_CHAR;
        Serial.write(out_str.c_str(), out_str.length());
    }
    else if (type_of_message == 2)
    {
        if (message == LS_SPONT)
        {
            out_str = LOCK_STATUS_MESSAGE + String(lock_state);
        }
        if (message == VIDREQ_SPONT)
        {
            out_str = VIDEO_REQUEST_MESSAGE;
        }
        out_str = SPONT_HEADER + out_str + TERMINATING_CHAR;
        Serial.write(out_str.c_str(), out_str.length());
    }
    out_str = "";
}
