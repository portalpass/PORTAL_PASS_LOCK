// LOCK RECEIVER CALLBACK CLASS
#ifndef RECEIVER_CALLBACK_H
#define RECEIVER_CALLBACK_H

#include <string>

#include "agora_sig.h"

using namespace agora_sdk_cpp;

// DECLARE CALLBACK CLASS AND INHERIT BASIC CALLBACK FUNCTIONS
class receiverCallBack : public agora_sdk_cpp::ICallBack
{
    // PRIVATE DATA MEMBERS:
    private:
    
    // STORE AGORA API INSTANCE
    IAgoraAPI* msgAPI;
    
    // SERIAL NUMBER OF RECEIVER
    std::string lock_serial;
    
    // STATUS FLAGS
    bool LOGIN_FLAG;
    bool SENT_FLAG;
    bool LOGOUT_FLAG;
    
    // PUBLIC FUNCTIONS:
    public:
    
    // API: INSTANCE OF AGORA API
    receiverCallBack( IAgoraAPI* API );
    
    ~receiverCallBack( void );
    // FUNCTION TO SET LOCK SERIAL
    void setSerial( const std::string &serial );
    
    void onLoginSuccess( uint32_t uid, int fd );
    
    void onLoginFailed( int ecode );

    bool getLoginFlag( );

    void onMessageSendSuccess( char const * messageID, size_t messageID_size);

    void onMessageSendError( char const * messageID, size_t messageID_size, int ecode );

    bool getSentFlag( );
    
    // SETS VALUE OF SENT_FLAG TO FALSE
    void resetSentFlag( );
    
    //react to receiving a message
    void onMessageInstantReceive( char const * account, size_t account_size, uint32_t uid, char const * msg, size_t msg_size );

    void onLogout( int ecode );
    
    bool getLogoutFlag( );
};

#endif
