//implementation file of senderCallBack.h
#include <cstdlib>
#include <string>
#include <iostream>

#include "senderCallBack.h"
#include "helper_functions.h"

senderCallBack::senderCallBack( IAgoraAPI* API )
{
    // STORE INSTANCE OF AGORA API
    msgAPI = API;
    
    // SET ALL FLAGS TO FALSE
    LOGIN_FLAG = false;
    SENT_FLAG = false;
    LOGOUT_FLAG = false;
}

// FUNCTION TO SET LOCK SERIAL NUMBER
void senderCallBack::setSerial( const std::string &serial )
{
    lock_serial = serial;
}

void senderCallBack::onLoginSuccess( uint32_t uid,int fd )
{
    LOGIN_FLAG = true;
    std::cout << "SENDER LOGGED IN!" << std::endl;
}

void senderCallBack::onLoginFailed( int ecode )
{
    std::cerr << "SENDER LOGIN ERROR: " << ecode << std::endl;
    std::exit( 1 );
}

bool senderCallBack::getLoginFlag( )
{
    return LOGIN_FLAG;
}

void senderCallBack::onMessageSendSuccess( char const * messageID, size_t messageID_size)
{
    SENT_FLAG = true;
    std::cout << "SENDER SIGNAL SENT!" << std::endl;
}

void senderCallBack::onMessageSendError( char const * messageID, size_t messageID_size, int ecode )
{
    std::cerr << "SENDER SIGNAL ERROR: " << ecode << std::endl;
    std::exit( 1 );
}

bool senderCallBack::getSentFlag( )
{
    return SENT_FLAG;
}

void senderCallBack::resetSentFlag( )
{
    SENT_FLAG = false;
}

void senderCallBack::onLogout( int ecode )
{
    // IF LOGOUT FAILED
    if( ecode != 101 )
    {
        // THROW EXCEPTION
        std::cerr << "SENDER LOGOUT ERROR: " << ecode << std::endl;
        std::exit( 1 );
    }
    else
    {
        // IF LOGOUT SUCCESS
        // SET LOGOUT_FLAG
        LOGOUT_FLAG = true;
        std::cout << "SENDER LOGGED OUT! " << std::endl;
    }
}

bool senderCallBack::getLogoutFlag( )
{
    return LOGOUT_FLAG;
}
