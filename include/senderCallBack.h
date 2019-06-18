// LOCK SENDER CALLBACK CLASS
#ifndef SENDER_CALLBACK_H
#define SENDER_CALLBACK_H

#include <string>

#include "agora_sig.h"

using namespace agora_sdk_cpp;

// DECLARE CALLBACK CLASS AND INHERIT BASIC CALLBACK FUNCTIONS
class senderCallBack : public agora_sdk_cpp::ICallBack
{
    // PRIVATE DATA MEMBERS:
    private:
    
    // STORE AGORA API INSTANCE
    IAgoraAPI* msgAPI;
    
    // SERIAL NUMBER OF SENDER
    std::string lock_serial;
    
    // STATUS FLAGS
    bool LOGIN_FLAG;
    bool SENT_FLAG;
    bool LOGOUT_FLAG;

    // PUBLIC FUNCTIONS:
    public:
    
    // API: INSTANCE OF AGORA API
    senderCallBack( IAgoraAPI* API );
    
    // FUNCTION TO SET LOCK SERIAL
    void setSerial( const std::string &serial );
        
    void onLoginSuccess( uint32_t uid,int fd );

    void onLoginFailed( int ecode );

    bool getLoginFlag( );

    void onMessageSendSuccess( char const * messageID, size_t messageID_size);

    void onMessageSendError( char const * messageID, size_t messageID_size, int ecode );
    
    bool getSentFlag( );
    
    // SETS VALUE OF SENT_FLAG TO FALSE
    void resetSentFlag( );
    
    void onLogout( int ecode );

    bool getLogoutFlag( );
    
};

#endif
