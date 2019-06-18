// LOCK'S RECEIVER PROGRAM
// LOGS IN THEN RUNS IN AN INFINITE WAITING LOOP.
// REACTS ASYNCHRONOUSLY WITH FUNCTIONS IN receiverCallBack.h and lockAPI.h
// Can reply to messages when asked but can't summon its own messages
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>

#include "helper_functions.h"
#include "lockAPI.h"
#include "receiverCallBack.h"
#include "agora_sig.h"

static const std::string python_dir = helper::get_python_dir();
static const std::string conf_dir   = helper::get_conf_dir();

// FUNCTION TO READ APPID AND THE RENTIRE LOCK sSERIAL NUMBER
// FROM "setup.txt" AND RETURN THEM TO MAIN
// IF A KEYPAIR NAMED LOCK_SERIAL DOESNT EXIST, MAKE ONE
int setupReceiver( std::string &APPID, std::string &LOCK_SERIAL )
{

    std::string setup_path = conf_dir + "/setup.txt";

    std::ifstream setup_f;
    setup_f.open( setup_path, std::ios_base::in );
    
    if( setup_f.fail( ) )
    {
        std::cerr << "ERROR: setup.txt not found." << std::endl;
        std::cerr << "\tExpected location: " << std::endl
                  << "\t" << setup_path << std::endl;
        return 1;
    } 
    // OBTAIN APPID, REMOVE "APPID=", & STORE IT
    std::getline( setup_f, APPID );
    APPID = APPID.substr( 6 );
    
    // OBTAIN SERIAL NUMBER & CHECK IF THE SERIAL NUMBER
    // IS ( 32 BYTES + 'SERIAL=' ) LONG
    LOCK_SERIAL.clear();
    std::getline( setup_f, LOCK_SERIAL );
    if( LOCK_SERIAL.size( ) != 39 )
    {
        std::cerr << "ERROR: setup.txt — Serial length invalid" << std::endl;
        return 2;
    }
    
    // REMOVE "SERIAL="
    LOCK_SERIAL = LOCK_SERIAL.substr( 7 );

    //Check if key_pair exists
    std::ifstream key_f ( ( conf_dir + "/" + LOCK_SERIAL + ".priv.pem").c_str() );
    if ( key_f )
    {
        key_f.close();
        return 0;
    }
    
    //Check if key_pair exists
    if ( helper::file_doesnt_exist( conf_dir + "/" + LOCK_SERIAL + ".pub.pem" ) || 
         helper::file_doesnt_exist( conf_dir + "/" + LOCK_SERIAL + ".pub.pem" ) )
    {
        std::cerr << "Generating keypair " << ( conf_dir + "/" + LOCK_SERIAL ) << std::endl;
        return helper::rsa_genkeypair( ( conf_dir + "/" + LOCK_SERIAL ).c_str() );
    }
    return 0;

}

// SETUP RECEIVER AND LEAVE IT RUNNING
int main( )
{
    // VARIABLES TO STORE APPID, RECEIVER_ID
    std::string APPID;
    std::string LOCK_SERIAL;
    
    // OBTAIN APPID & RECEIVER_ID
    if ( int error_code = setupReceiver( APPID, LOCK_SERIAL ) )
    {
        return error_code;
    }
    
    std::string RECEIVER_SERIAL = LOCK_SERIAL.substr( 0, 32 );
    // OUTPUT THE RECEIVER_ID
    std::cout << "RECEIVER: " << LOCK_SERIAL << std::endl;

    // DECLARE LOCK API INSTANCE FOR A RECEIVER
    lockAPI<receiverCallBack> API( APPID, LOCK_SERIAL );
    
    
    // LOG INTO THE AGORA COMM SYSTEM
    API.login( RECEIVER_SERIAL );
    
    // LISTEN FOR INCOMING MESSAGES
    
    //std::string cmd;
    while( true ) std::this_thread::sleep_for( std::chrono::seconds ( 1 ) ); 

    
    return 0;
}
