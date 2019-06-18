//implementation file of receiverCallBack.h
//Any communication to hardware uses ZMQ to connect to a python program
//The python program MUST be running to function correctly
//Curently it is script/arduino_test.py
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include <poll.h>
#include <spawn.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <zmq.hpp>

#include "receiverCallBack.h"
#include "helper_functions.h"

extern char **environ;

static const int HEADER_LENGTH   = 6;
static const int POLL_TIMEOUT_MS = 10*1000;

static const std::string sender_Header    = "S";
static const std::string receiver_Header  = "R";
static const std::string android_Header   = "A";
static const std::string encrypt_Header   = "E";
static const std::string unencrypt_Header = "U";

//receive from android forward to ZMQ
static const std::string QR_REQUEST           = "GETQR:";
static const std::string QR_REPLY             = "RPLQR:";

static const std::string UNLOCK_REQUEST        = "UNLOCK";
static const std::string LOCK_REQUEST          = "LOCK!!";
static const std::string DOOR_STATE_REQUEST    = "DOORS?";
static const std::string LOCK_STATE_REQUEST    = "LOCKS?";

static const std::string SET_LOCK_TIMEOUT_MS   = "SETTI:";
static const std::string QUERY_LOCK_TIMEOUT_MS = "LOCTO?";

static const std::string SET_LED_REQUEST      = "SETLED";
static const std::string LOW_LED_REQUEST      = "LOWLED";
static const std::string QUERY_LED_REQUEST    = "QUERYL";

static const std::string VIDEO_REQUEST        = "VIDBEG";
static const std::string VIDEO_END            = "VIDEND";

//strings to send to zmq
static const std::string unlock_zmq           = "unl";
static const std::string lock_zmq             = "loc";
static const std::string door_state_zmq       = "?ds";
static const std::string lock_state_zmq       = "?ls";

static const std::string set_led_zmq          = "hl1";
static const std::string low_led_zmq          = "ll1";
static const std::string query_led_zmq        = "?l1";

static const std::string set_locktimeout_zmq   = "sT";
static const std::string query_locktimeout_zmq = "?lT";

//stuff
static const std::string python_dir = helper::get_python_dir();
static const std::string conf_dir   = helper::get_conf_dir();
static const std::string home_dir   = helper::get_home_dir();

//send a hardware request to the python serial port owner script, and
//wait for its reply to forward back to the requester.
static std::string zmq_to_python ( const std::string &to_send )
{
    zmq::context_t context( 1 );
    zmq::socket_t  socket( context, ZMQ_REQ );
    zmq_connect( socket, "tcp://localhost:5555" );
    
    std::cerr << "Forwarding request to python .... " << std::endl;
    
    char buff[ 256 ];
    
    zmq_send( socket, to_send.data(), to_send.size(), 0 );
    
    int bytes = zmq_recv( socket, buff, 256, 0 );
    if ( bytes <= 0 )
    {
        std::cerr << "ERROR" << std::endl;
        return "";
    }

    std::string from_python( buff, bytes );

    return from_python;
    
}

receiverCallBack::receiverCallBack( IAgoraAPI* API )
{

    //Kill processes that may be running
    system("pkill zbarcam");
    system("pkill scanQR.sh");
    system("pkill verify_QR.py");
    system("pkill nw" );
    system("pkill lockControl.py");
    system("pkill startx");

    // STORE INSTANCE OF AGORA API
    msgAPI = API;
    
    // SET ALL FLAGS TO FALSE
    LOGIN_FLAG = false;
    SENT_FLAG = false;
    LOGOUT_FLAG = false;

    //START ZBAR & lockControl
    pid_t childpid;
    std::string tmp = "startx ~/PORTAL_PASS_TEST/config/xinitzb"; 

    char *start_zbar[ 4 ];
    start_zbar[ 0 ] = const_cast<char*>( "/bin/bash" );
    start_zbar[ 1 ] = const_cast<char*>( "-c" );
    start_zbar[ 2 ] = const_cast<char*>( tmp.c_str() );
    start_zbar[ 3 ] = NULL;
    int status = posix_spawn( &childpid, "/bin/bash", NULL, NULL, start_zbar, environ );

    if ( status )
    {
        std::cerr << "Error starting zbar" << std::endl;
    }

    tmp = (std::string) "cd " + python_dir + (std::string)"/lock_control && ./lockControl.py";
    char *start_lockControl[ 4 ];
    start_lockControl[ 0 ] = const_cast<char*>( "/bin/bash" );
    start_lockControl[ 1 ] = const_cast<char*>( "-c" );
    start_lockControl[ 2 ] = const_cast<char*>( tmp.c_str() );
    start_lockControl[ 3 ] = NULL;
    status = posix_spawn( &childpid, "/bin/bash", NULL, NULL, start_lockControl, environ );
    if ( status )
    {
        std::cerr << "Error starting lockControl.py" << std::endl;
    }
    std::cerr << "receiver opening. Spawned lockControl and scanQR" << std::endl;
}

receiverCallBack::~receiverCallBack( void )
{
    std::cerr << "receiver closing. Killing extra processes" << std::endl;
    system("pkill zbarcam");
    system("pkill scanQR.sh");
    system("pkill verify_QR.py");
    system("pkill nw" );
    system("pkill lockControl.py");
    system("pkill startx");
}

// FUNCTION TO SET LOCK SERIAL NUMBER
void receiverCallBack::setSerial( const std::string &serial )
{
    lock_serial = serial;
}

void receiverCallBack::onLoginSuccess( uint32_t uid, int fd )
{
    LOGIN_FLAG = true;
    std::cout << "RECEIVER LOGGED IN!" << std::endl;
}

void receiverCallBack::onLoginFailed( int ecode )
{
    std::cerr << "RECEIVER LOGIN ERROR: " << ecode << std::endl;
    std::exit( 1 );
}

bool receiverCallBack::getLoginFlag( )
{
    return LOGIN_FLAG;
}

void receiverCallBack::onMessageSendSuccess( char const * messageID, size_t messageID_size)
{
    SENT_FLAG = true;
    std::cout << "RECEIVER SIGNAL SENT!" << std::endl;
}

void receiverCallBack::onMessageSendError( char const * messageID, size_t messageID_size, int ecode )
{
    std::cout << "RECEIVER SIGNAL ERROR: " << ecode << std::endl;
}

bool receiverCallBack::getSentFlag( )
{
    return SENT_FLAG;
}

void receiverCallBack::resetSentFlag( )
{
    SENT_FLAG = false;
}

//take an action based on receiving a message
void receiverCallBack::onMessageInstantReceive( char const * account, size_t account_size, uint32_t uid, char const * msg, size_t msg_size )
{

    //Decrypts message, then either runs if it is headered as a command, else prints the 
    //message to the terminal

    std::string signal( msg, msg_size );
    
    if ( signal.size() < 3 )
    {
        std::cerr << "Message is too short including headers" << std::endl;
        return;
    }

    std::string sender_serial( account, account_size );
    
    if ( signal.substr( 0, 1 ) == sender_Header )
    {
        sender_serial = sender_serial.substr( 16, 16 ) + sender_serial.substr( 0, 16 );
    }

    if( signal.substr( 0, 1 ) == sender_Header || signal.substr( 0, 1 ) == receiver_Header 
         || signal.substr( 0, 1 ) == android_Header )
    {
        signal = signal.substr( 1 );
    }
    else
    {
        std::cerr << "Received message with an invalid header " << std::endl;
        return;
    }

    //if encrypted, strip off, decode from base64, then check conditions
    if ( signal.substr( 0, 1 ) == encrypt_Header )
    {
        
        signal = signal.substr( 1 );

        std::string pubk_f  = conf_dir + "/" + sender_serial + ".pub.pem";
        std::string privk_f = conf_dir + "/" + lock_serial + ".priv.pem";

        signal = helper::base64_decode( signal );

        std::string signal_decrypt;

        if ( helper::rsa_decrypt_verify( signal, pubk_f, privk_f, signal_decrypt ) )
        {
            std::cerr << "Error decrypting OR verifying message" << std::endl;
            return;
        }


        std::cerr << "Encrypted Message Received" << std::endl
                  << (char)0x1B << "[1m" 
                  << sender_serial << ": " << signal_decrypt 
                  << (char)0x1B << "[0m" << std::endl;
    
        //All messages has a 6 byte header, then some have optional arguments after
        //parse those off
        std::string HEADER;
        if( signal_decrypt.length() >= HEADER_LENGTH )
        {
            HEADER = signal_decrypt.substr(0,HEADER_LENGTH);
            signal_decrypt = signal_decrypt.substr( HEADER_LENGTH );
        }
        else
        {
            std::cerr << "Header length is too short to be a recognized command" << std::endl;
            return;
        }

        //test, if asked for a QR code, make one and send it back to the sender
        //Android will request conditions that will be forwarded to python to put in the database
        //QRREQ:<Conditions>
        if ( HEADER == QR_REQUEST )
        {
            std::string qr_string;

            std::cout << "Generating QR code with conditions " << signal_decrypt << std::endl;

            int success = helper::exec( " cd " + python_dir + "/qr_scripts && ./generate_QR.py " + signal_decrypt, qr_string );
            
            if ( qr_string == "" )
            {
                std::cerr << "Error generating QR code ... " << std::endl;
                return;
            }

            std::string to_encrypt = QR_REPLY + qr_string;
            std::cerr << "Sending QR Code to " << sender_serial << std::endl;

            std::string s;
            if ( helper::rsa_encrypt_sign( to_encrypt, pubk_f, privk_f, s ) ) 
            {
                std::cerr << "error encrypting " << std::endl;
                return;
            }
            s = helper::base64_encode( reinterpret_cast<unsigned char const*>( s.data() ) , s.size() );

            s = receiver_Header + encrypt_Header + s;

            msgAPI->messageInstantSend( sender_serial.data(), sender_serial.size(), 0, s.data(), s.size(), NULL, 0 );
        }

        //if recieved a QR code, decode from base64 and store.
        //analog for android displaying the png
        //this isn't needed for real and is only for testing
        else if ( HEADER == QR_REPLY )
        {
            
            std::string qr_string = signal_decrypt;

            std::cerr << "Received QR Code! Deconding and storing in " +conf_dir + "/QR.png" << std::endl;
            std::string s = helper::base64_decode( qr_string );
            
            std::fstream out_f( ( conf_dir + "/QR.png" ).c_str(), std::ios::out | std::ios::binary );
            
            if ( out_f.fail() )
            {
                std::cout << "Error opening file for writing " << std::endl;
            }
            else
            {
                out_f.write( s.c_str(), s.size() );
                out_f.close();
            }
        }

        else if ( HEADER == SET_LED_REQUEST )
        {
            std::string from_python = zmq_to_python( set_led_zmq );
            if ( from_python == "" )
            {
                std::cerr << "ERROR" << std::endl;
                return;
            }
            std::cerr << "Sending result \"" << from_python << "\" back to Requester" << std::endl;

            std::string s;
            if ( helper::rsa_encrypt_sign( from_python, pubk_f, privk_f, s ) ) 
            {
                std::cerr << "error encrypting " << std::endl;
                return;
            }
            s = helper::base64_encode( reinterpret_cast<unsigned char const*>( s.data() ) , s.size() );

            s = receiver_Header + encrypt_Header + s;

            msgAPI->messageInstantSend( sender_serial.data(), sender_serial.size(), 0, s.data(), s.size(), NULL, 0 );
            return;
        }

        else if ( HEADER == LOW_LED_REQUEST )
        {
            std::string from_python = zmq_to_python( low_led_zmq );
            if ( from_python == "" )
            {
                std::cerr << "ERROR" << std::endl;
                return;
            }
            std::cerr << "Sending result \"" << from_python << "\" back to " << sender_serial << std::endl;
            
            std::string s;
            if ( helper::rsa_encrypt_sign( from_python, pubk_f, privk_f, s ) ) 
            {
                std::cerr << "error encrypting " << std::endl;
                return;
            }
            s = helper::base64_encode( reinterpret_cast<unsigned char const*>( s.data() ) , s.size() );

            s = receiver_Header + encrypt_Header + s;

            msgAPI->messageInstantSend( sender_serial.data(), sender_serial.size(), 0, s.data(), s.size(), NULL, 0 );
            return;
        }

        else if ( HEADER == QUERY_LED_REQUEST )
        {
            std::string from_python = zmq_to_python( query_led_zmq);
            if ( from_python == "" )
            {
                std::cerr << "ERROR" << std::endl;
                return;
            }
            std::cerr << "Sending result \"" << from_python << "\" back to " << sender_serial << std::endl;

            std::string s;
            if ( helper::rsa_encrypt_sign( from_python, pubk_f, privk_f, s ) ) 
            {
                std::cerr << "error encrypting " << std::endl;
                return;
            }
            s = helper::base64_encode( reinterpret_cast<unsigned char const*>( s.data() ) , s.size() );

            s = receiver_Header + encrypt_Header + s;

            msgAPI->messageInstantSend( sender_serial.data(), sender_serial.size(), 0, s.data(), s.size(), NULL, 0 );
            return;
        }

        else if ( HEADER == UNLOCK_REQUEST)
        {
            std::string from_python = zmq_to_python( unlock_zmq );
            if ( from_python == "" )
            {
                std::cerr << "ERROR" << std::endl;
                return;
            }
            std::cerr << "Sending result \"" << from_python << "\" back to " << sender_serial << std::endl;
            
            std::string s;
            if ( helper::rsa_encrypt_sign( from_python, pubk_f, privk_f, s ) ) 
            {
                std::cerr << "error encrypting " << std::endl;
                return;
            }
            s = helper::base64_encode( reinterpret_cast<unsigned char const*>( s.data() ) , s.size() );

            s = receiver_Header + encrypt_Header + s;

            msgAPI->messageInstantSend( sender_serial.data(), sender_serial.size(), 0, s.data(), s.size(), NULL, 0 );
            return;
        }

        else if ( HEADER == LOCK_REQUEST )
        {
            std::string from_python = zmq_to_python( lock_zmq );
            if ( from_python == "" )
            {
                std::cerr << "ERROR" << std::endl;
                return;
            }
            std::cerr << "Sending result \"" << from_python << "\" back to " << sender_serial << std::endl;
            
            std::string s;
            if ( helper::rsa_encrypt_sign( from_python, pubk_f, privk_f, s ) ) 
            {
                std::cerr << "error encrypting " << std::endl;
                return;
            }
            s = helper::base64_encode( reinterpret_cast<unsigned char const*>( s.data() ) , s.size() );

            s = receiver_Header + encrypt_Header + s;

            msgAPI->messageInstantSend( sender_serial.data(), sender_serial.size(), 0, s.data(), s.size(), NULL, 0 );
            return;
        }

        else if ( HEADER == DOOR_STATE_REQUEST)
        {
            std::string from_python = zmq_to_python( door_state_zmq );
            if ( from_python == "" )
            {
                std::cerr << "ERROR" << std::endl;
                return;
            }
            std::cerr << "Sending result \"" << from_python << "\" back to Requester" << std::endl;
            
            std::string s;
            if ( helper::rsa_encrypt_sign( from_python, pubk_f, privk_f, s ) ) 
            {
                std::cerr << "error encrypting " << std::endl;
                return;
            }
            s = helper::base64_encode( reinterpret_cast<unsigned char const*>( s.data() ) , s.size() );

            s = receiver_Header + encrypt_Header + s;

            msgAPI->messageInstantSend( sender_serial.data(), sender_serial.size(), 0, s.data(), s.size(), NULL, 0 );
            return;
        }

        else if ( HEADER == LOCK_STATE_REQUEST)
        {
            std::string from_python = zmq_to_python( lock_state_zmq );
            if ( from_python == "" )
            {
                std::cerr << "ERROR" << std::endl;
                return;
            }
            std::cerr << "Sending result \"" << from_python << "\" back to Requester" << std::endl;
            
            std::string s;
            if ( helper::rsa_encrypt_sign( from_python, pubk_f, privk_f, s ) ) 
            {
                std::cerr << "error encrypting " << std::endl;
                return;
            }
            s = helper::base64_encode( reinterpret_cast<unsigned char const*>( s.data() ) , s.size() );

            s = receiver_Header + encrypt_Header + s;

            msgAPI->messageInstantSend( sender_serial.data(), sender_serial.size(), 0, s.data(), s.size(), NULL, 0 );
        }   
        
        else if ( HEADER == SET_LOCK_TIMEOUT_MS )
        {
            std::cerr << "Setting lock timeout to " << signal_decrypt << "ms" << std::endl;
            std::string from_python = zmq_to_python( set_locktimeout_zmq + signal_decrypt );
            if ( from_python == "" )
            {
                std::cerr << "ERROR" << std::endl;
                return;
            }
            std::cerr << "Sending result \"" << from_python << "\" back to Requester" << std::endl;
            
            std::string s;
            if ( helper::rsa_encrypt_sign( from_python, pubk_f, privk_f, s ) ) 
            {
                std::cerr << "error encrypting " << std::endl;
                return;
            }
            s = helper::base64_encode( reinterpret_cast<unsigned char const*>( s.data() ) , s.size() );

            s = receiver_Header + encrypt_Header + s;

            msgAPI->messageInstantSend( sender_serial.data(), sender_serial.size(), 0, s.data(), s.size(), NULL, 0 );
        }

        else if ( HEADER == QUERY_LOCK_TIMEOUT_MS)
        {
            std::string from_python = zmq_to_python( query_locktimeout_zmq );
            if ( from_python == "" )
            {
                std::cerr << "ERROR" << std::endl;
                return;
            }
            std::cerr << "Sending result \"" << from_python << "\" back to Requester" << std::endl;
            
            std::string s;
            if ( helper::rsa_encrypt_sign( from_python, pubk_f, privk_f, s ) ) 
            {
                std::cerr << "error encrypting " << std::endl;
                return;
            }
            s = helper::base64_encode( reinterpret_cast<unsigned char const*>( s.data() ) , s.size() );

            s = receiver_Header + encrypt_Header + s;

            msgAPI->messageInstantSend( sender_serial.data(), sender_serial.size(), 0, s.data(), s.size(), NULL, 0 );
        }

        //end ZBAR and start video
        //Android will send channel name and password which iwll be written to a temporary file
        //VIDBEG:<channel_name>:<password>
        else if( HEADER == VIDEO_REQUEST )
        {
            std::string check_for_nw = "pgrep nw";
            std::string result = "a";
            
            if( helper::exec( check_for_nw, result ) != 0 && result != "" )
            {
                return;
            }
            
            std::ofstream out;
            std::string channelInfo = home_dir + "/PORTAL_PASS_TEST/video/channelInfo.js";
            
            out.open( channelInfo, std::ios_base::out | std::ios_base::trunc );
            
            if( out.fail() ){ return; }
            
            int passwordStart = signal_decrypt.find( ':', 1 );
            if ( passwordStart == std::string::npos )
            {
                return;
            }

            out << "var ChannelName = \"" << signal_decrypt.substr( 1, passwordStart - 1 );
            out << "\";" << std::endl;
            out << "var Password = \"" << signal_decrypt.substr( passwordStart + 1 ) << "\";" << std::endl;
            
            out.close( );
            
            pid_t childpid;
           
            std::string tmp = "startx ~/PORTAL_PASS_TEST/config/xinitnw";
            char *start_nw[ 4 ];
            start_nw[ 0 ] = const_cast<char*>( "/bin/bash" );
            start_nw[ 1 ] = const_cast<char*>( "-c" );
            start_nw[ 2 ] = const_cast<char*>( tmp.c_str() );
            start_nw[ 3 ] = NULL;
            
            char *pkill_zbar[ 4 ];
            pkill_zbar[ 0 ] = const_cast<char*>( "/bin/bash" );
            pkill_zbar[ 1 ] = const_cast<char*>( "-c" );
            pkill_zbar[ 2 ] = const_cast<char*>( "pkill zbarcam; pkill scanQR; pkill verify_QR.py; pkill startx" );
            pkill_zbar[ 3 ] = NULL;
            
            int status;

            status = posix_spawn( &childpid, "/bin/bash", NULL, NULL, pkill_zbar, environ );

            if ( status == 0 )
            {
                std::cout << "Terminating zbar..." << std::endl;
                
                std::string check_for_zbar = "pgrep zbarcam";
                std::string tmp = "1";

                do 
                {
                    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
                } while( helper::exec( check_for_zbar, tmp ) != 0 && tmp != "");

                status = posix_spawn( &childpid, "/bin/bash", NULL, NULL, start_nw, environ );
                
                if ( status == 0 )
                {
                    std::cout << "Launching video call..." << std::endl;
                }
            }
            
            return;
        }

        //end video and resume ZBAR
        else if( HEADER == VIDEO_END )
        {
            //if received and video is over already do nothing
            if ( system( "pgrep zbarcam" ) == 0 && system( "pgrep nw" ) != 0 )
            {
                return;
            }
            
            //delete the channel name / password file
            system( "rm -f ~/PORTAL_PASS_TEST/video/channelInfo.js" );

            pid_t childpid;
            
            std::string tmp = "startx ~/PORTAL_PASS_TEST/config/xinitzb"; 

            char *start_zbar[ 4 ];
            start_zbar[ 0 ] = const_cast<char*>( "/bin/bash" );
            start_zbar[ 1 ] = const_cast<char*>( "-c" );
            start_zbar[ 2 ] = const_cast<char*>( tmp.c_str() );
            start_zbar[ 3 ] = NULL;
            
            char *pkill_nw[ 4 ];
            pkill_nw[ 0 ] = const_cast<char*>( "/bin/bash" );
            pkill_nw[ 1 ] = const_cast<char*>( "-c" );
            pkill_nw[ 2 ] = const_cast<char*>( "pkill nw; pkill startx" );
            pkill_nw[ 3 ] = NULL;
            
            int status;
            
            status = posix_spawn( &childpid, "/bin/bash", NULL, NULL, pkill_nw, environ );

            if ( status == 0 )
            {
                std::cout << "Terminating nw..." << std::endl;
                
                // SLEEP TO GIVE TIME FOR THE SERVER TO COMPLETELY DETACH ITSELF
                // FROM THE CAMERA

                std::string check_for_nw = "pgrep nw";
                std::string tmp = "a";

                do
                {
                    std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
                } while( helper::exec( check_for_nw, tmp ) != 0 && tmp != "" );

                std::cout << "Launching zbarcam..." << std::endl;
                status = posix_spawn( &childpid, "/bin/bash", NULL, NULL, start_zbar, environ );
            }
            
            return;
        }
        return;
    }

    else if ( signal.substr( 0, 1 ) == unencrypt_Header )
    {
        signal = signal.substr( 1 );
    }

    else
    {
        std::cerr << "HEADER NOT RECOGNIZED" << std::endl;
        return;
    }
    
    //If someone sends its public key, Ask user to confirm and import it
    //as trusted, then send out their own public key back
    if ( signal.find( "-----BEGIN PUBLIC KEY-----") == 0
         && signal.find( "-----END PUBLIC KEY-----") != std::string::npos )
    {
        std::string response;
        
        //flush out cin
        //while ( std::getline(std::cin, response ) ) {}
        //response = "";

        std::cerr << "RECEIVED PUBLIC KEY FROM " << sender_serial << std::endl
                  << "IMPORT? (y/n)" << std::endl
                  << ">" << std::flush;

        struct pollfd pfd = { STDIN_FILENO, POLLIN, 0 };
        int ret = 0;
        ret = poll(&pfd, 1, POLL_TIMEOUT_MS); 
        if(ret == 1) // there is something to read
        {
            std::getline(std::cin, response);
        }
        else
        {
            std::cerr << "READ TIMED OUT" << std::endl;
            response = "NNNNNNN";
        }

        //if accept, store in account_name.pub.pem and then send my pubkey back
        if ( response [ 0 ] == 'y' || response[ 0 ] == 'Y' )
        {
            std::string store_in = conf_dir + "/" + sender_serial + ".pub.pem";
           
            std::string list_path = conf_dir + "/paired_serials.txt"; 

            std::fstream out_f( ( store_in ).c_str(), std::ios::out | std::ios::binary );
            std::fstream list_f( list_path.c_str(), std::ios::app );

            std::cerr << "Importing key ... " << std::endl;

            if ( out_f.fail() || list_f.fail() )
            {
                std::cerr << "Error opening file for writing key " << std::endl;
            }
            else
            {
                out_f.write( signal.c_str(), signal.size() );
                out_f.close();
                list_f.write( ( sender_serial +"\n").c_str(), sender_serial.size() + 1 );
            }
            
            //read in your public key and send it back
            std::fstream key_send_f( ( conf_dir + "/" + lock_serial + ".pub.pem" ).c_str(), std::ios::in );

            if ( key_send_f.fail() )
            {
                std::cerr << "Failed to open key file to send back" << std::endl;
                return;
            }

            std::string s( ( std::istreambuf_iterator<char>( key_send_f ) ),
                             std::istreambuf_iterator<char>() );
            key_send_f.close();

            s = receiver_Header + unencrypt_Header + s;

            std::cerr << "Sending key back to " << sender_serial << std::endl;

            msgAPI->messageInstantSend( sender_serial.data(), sender_serial.size(), 0, s.data(), s.size(), NULL, 0 );

        }
        else
        {
            std::cerr << "Key rejected!" << std::endl;
        }
        return;
    }

    //if received unencrypted message, print it but do not execute it / value any of its headers
    std::cerr << "Unencrypted message received" << std::endl
              << (char)0x1B << "[1m" 
              << sender_serial << ": " << signal 
              << (char)0x1B << "[0m" << std::endl;

}

void receiverCallBack::onLogout( int ecode )
{
    // IF LOGOUT FAILED
    if( ecode != 101 )
    {
        // THROW EXCEPTION
        std::cout << "RECEIVER LOGOUT ERROR: " << ecode << std::endl;
        std::exit( 1 );
    }
    else
    {
        // IF LOGOUT SUCCESS
        // SET LOGOUT_FLAG
        LOGOUT_FLAG = true;
        std::cout << "RECEIVER LOGGED OUT! " << std::endl;
    }
}

bool receiverCallBack::getLogoutFlag( )
{
    return LOGOUT_FLAG;
}
