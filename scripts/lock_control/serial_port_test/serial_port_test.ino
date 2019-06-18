#define LED1           7
#define VIDEO_REQUEST  8
#define DOOR_STATUS   10 

#define INPUT_LENGTH      3
#define CLEAR_CHAR       '*'
#define TERMINATING_CHAR 'X'

#define LED_ON    "hl1"
#define LED_OFF   "ll1"
#define UNLOCK    "unl"

#define QUERY_LED "?l1"
#define QUERY_DS  "?ds"
#define QUERY_LS  "?ls"

#define SPONT_HEADER "SPONT:"
#define REPLY_HEADER "REPLY:"

String in_s;
String out_s;

int l1;
int ds;

int vid_request;
int old_vid_request;
long long int time_vid_request;
long long int vid_request_timeout_ms;

int want_to_unlock;
int unlocked;
long long int time_unlocked;
long long int lock_timeout_ms;

void unlock ()
{
  
}

void lock ()
{
  
}

void setup() 
{

    in_s  = "";
    out_s = "";

    ds              = 0;
    l1              = 0;

    vid_request     = 0;
    old_vid_request = 0;

    want_to_unlock  = 0;
    unlocked        = 0;
    lock_timeout_ms = 5000;
    time_unlocked   = -1 * lock_timeout_ms;

    vid_request_timeout_ms = 5000;
    time_vid_request       = -1 * vid_request_timeout_ms;
   
    pinMode( LED1         , OUTPUT );
    pinMode( DOOR_STATUS  , INPUT  );
    pinMode( VIDEO_REQUEST, INPUT  );

    //Wrie / use arduino pulldown resistor on each pin
    digitalWrite( LED1         , 0 );
    digitalWrite( DOOR_STATUS  , 0 );
    digitalWrite( VIDEO_REQUEST, 0 );

    ds = digitalRead( DOOR_STATUS );
    if ( ds == 0 )
    {
        lock();
    }

    Serial.begin(9600);
    Serial.flush();

}

void loop() 
{
    //read up to input length characters
    //if a special character is recieved, clear the contents of S
    //Therefore, Computer should send  CLEARL_CHARACTER + INPUT each time to ensure it is cleared
    //the advantage of this asynchronous reading is to always read the message in INPUT_LENGTH Blocks, and to 
    //allow the arduino to do other things while its waiting for characters
    
    while( Serial.available() && in_s.length() < INPUT_LENGTH  )
    {
        char char_in = (char) Serial.read();
        if ( char_in  == CLEAR_CHAR || char_in == '\n' )
        {
            in_s = "";
        }
        else
        {
            in_s = in_s + char_in;
        }
    }

    //do a thing based on what the characters are, then clear the string
    if ( in_s.length() == INPUT_LENGTH ) 
    {
        //COMMANDS
        if ( in_s.equals( LED_ON ) )
        {
           digitalWrite( LED1, HIGH );
           l1 = 1;
        }
        else if ( in_s.equals( LED_OFF ) )
        {
           digitalWrite( LED1, LOW ); 
           l1 = 0;
        }   
        else if ( in_s.equals( UNLOCK ) )
        {
            want_to_unlock = 1;
        }
        
        //Query
        //1 is on, 0 is off?
        else if ( in_s.equals( QUERY_LED ) )
        {
            out_s = out_s + REPLY_HEADER + "STATUS LED1 = " + String( l1 ) + TERMINATING_CHAR;
        }
        //1 is open, 0 is closed?
        else if ( in_s.equals( QUERY_DS ) )
        {
            out_s = out_s + REPLY_HEADER + "STATUS DOOR = " + String( ds ) + TERMINATING_CHAR;
        }
        //1 is unlocked, 0 is locked?
        else if ( in_s.equals( QUERY_LS ) )
        {
            out_s = out_s + REPLY_HEADER + "STATUS LOCK = " + String( unlocked ) + TERMINATING_CHAR;
        }

        in_s = "";
        
    }

    ds = digitalRead( DOOR_STATUS );

    //Check if there is a video call request
    vid_request = digitalRead( VIDEO_REQUEST );

    if ( vid_request == 1 && old_vid_request == 0 && millis() > time_vid_request + vid_request_timeout_ms )
    {
        time_vid_request = millis();
        out_s = out_s + SPONT_HEADER + "VIDREQ" + TERMINATING_CHAR;  
    }

    old_vid_request = vid_request;

    //If we want to unlock, check door is closed and then do it
    if ( want_to_unlock == 1 && ds == 0 )
    {
        time_unlocked  = millis();
        want_to_unlock = 0;
        unlocked       = 1;
        unlock();
    }

    //Autolock timeout
    if ( millis() >= time_unlocked + lock_timeout_ms && ds == 0 )
    {
        unlocked = 0;
        lock();
    }

    //print output response back to send feedback to android
    //OUTPUTS of the ARDUINO will start with the SPONT_HEADER or REPLY_HEADER
    //Based on if the event was a response to a query or happened freely
    //each message will end with TERMINATING_CHARACTER
    //Multiple messages can be contained at a time in out_s,
    //python will parse at each TERMINATING_CHARACTER
    if ( out_s.length() > 0 )
    {
        Serial.write( out_s.c_str(), out_s.length() );
        out_s = "";
    }
}
