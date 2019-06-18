// LOCK'S SPONTANEOUS SENDER PROGRAM
// Options to send an encrypted or unencrypted message, or send pgp key
// Will be used to send staus updates
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "helper_functions.h"
#include "lockAPI.h"
#include "senderCallBack.h"
#include "agora_sig.h"

static const std::string python_dir = helper::get_python_dir();
static const std::string conf_dir   = helper::get_conf_dir();

// FUNCTION TO READ APPID AND THE ENTIRE LOCK SERIAL NUMBER
// FROM "setup.txt" AND RETURN THEM TO MAIN
// IF A KEYPAIR NAMED LOCK_SERIAL DOESNT EXIST, MAKE ONE
int setupSender( std::string &APPID, std::string &LOCK_SERIAL )
{

    std::string setup_path = conf_dir +"/setup.txt";

    std::ifstream setup_f;
    setup_f.open( setup_path, std::ios_base::in );
    
    if( setup_f.fail( ) )
    {

        std::cerr << "ERROR: setup.txt not found." << std::endl
                  << "\tExpected location: "<< std::endl
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

    setup_f.close( );
    
    //Check if key_pair exists
    if ( helper::file_doesnt_exist( conf_dir + "/" + LOCK_SERIAL + ".pub.pem" ) || 
         helper::file_doesnt_exist( conf_dir + "/" + LOCK_SERIAL + ".pub.pem" ) )
    {
        std::cerr << "Generating keypair " << ( conf_dir + "/" + LOCK_SERIAL ) << std::endl;
        return helper::rsa_genkeypair( ( conf_dir + "/" + LOCK_SERIAL ).c_str() );
    }
    return 0;
}

// SETUP THE SPONTANEOUS SENDER, LOGIN, SEND, LOGOUT
// INTENDED TO TAKE A RECEIVER ID, AND A SIGNAL OR MESSAGE
int main( int argc, char* argv[] )
{
    // CHECK FOR CORRECT NUMBER OF ARGS
    std::string to, msg;
    int e = 1;
   
    // VARIABLES TO STORE APPID, SENDER_ID
    std::string APPID;
    std::string LOCK_SERIAL;
    
    // OBTAIN APPID & SENDER_ID
    if ( int error_code = setupSender( APPID, LOCK_SERIAL ) )
    {
        std::cerr << "Error seting up" << std::endl;
        return error_code;
    }
    
    if ( argc == 2 ) 
    {
        to = "";
        msg = argv[ 1 ];
    }

    else if ( argc == 3 && std::string ( argv[ 2 ] ) == "send_pub_key" )
    {
        to  = argv[ 1 ];
        e = 0;
        
        std::fstream key_send_f( ( conf_dir + "/" + LOCK_SERIAL + ".pub.pem" ).c_str(), std::ios::in );

        if ( key_send_f.fail() )
        {
            std::cerr << "Failed to open key file to send back" << std::endl;
            return 4;
        }

        std::string temp( ( std::istreambuf_iterator<char>( key_send_f ) ),
                            std::istreambuf_iterator<char>() );
        msg = temp;
        key_send_f.close();
    }

    else
    {
        std::cerr << "SYNTAX" << std::endl
                  << "Send public key" << std::endl
                  << "\tlockSender <destination ID> send_pub_key" << std::endl
                  << "Send encrypted message to all serials in"
                  << conf_dir << "/paired_serials.txt" << std::endl
                  << "\tlockSender <signal_to_encrypt>" << std::endl;
        return 1; 
    }

    std::vector< std::string > to_v;
    if ( to == "" )
    {
        std::fstream paired_f( conf_dir + "/paired_serials.txt", std::ios::in );
        std::string temp;
        while ( std::getline( paired_f,temp ) )
        {
            if ( temp[ 0 ] != '#' )
            {
                to_v.push_back( temp  );
            }
        }
        paired_f.close();
    }

    else
    {
        to_v.push_back( to );
    }

    if ( to_v.size() == 0 )
    {
        std::cerr << "ERROR, no target specified" << std::endl
                  << "Verify the contents of " << conf_dir << "/paired_serials.txt" << std::endl
                  << "That it has at least 1 uncommented serial" << std::endl;
        return 7;
    }

    //The sender will login in with the first 16 bytes and the second 16 bytes of its serial swapped
    //to avoid collision

    std::string SENDER_SERIAL = LOCK_SERIAL.substr( 16, 16 ) + LOCK_SERIAL.substr( 0, 16 );
    
    // DECLARE LOCK API INSTANCE FOR A SENDER
    lockAPI<senderCallBack> API( APPID, LOCK_SERIAL );

    // LOG INTO THE AGORA COMM SYSTEM
    API.login( SENDER_SERIAL );
    
    // WAIT UNTIL SUCCESSFUL LOGIN
    while( !API.getLoginFlag() ){}

    // SEND SIGNAL TO INTENDED RECEIVER
    for ( int i = 0; i < to_v.size(); i++ )
    {
        std::cerr << "Sending to " << to_v[ i ] << std::endl;
        if ( int error_code = API.sendSignal( to_v[ i ], e, msg ) )
        {
            API.logout( );
            return error_code;
        }
    }
    // WAIT UNTIL SUCCESSFULLY SENT
    while( !API.getSentFlag() ){}
    
    // LOG OUT FROM THE AGORA COMM SYSTEM
    API.logout( );
    
    // WAIT UNIL SUCCESSFUL LOGOUT
    while( !API.getLogoutFlag( ) ){}
    
    return 0;
}
