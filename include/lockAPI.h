// GENERAL PURPOSE API FOR THE LOCK RECEIVER AND SENDER
#ifndef LOCKAPI_H
#define LOCKAPI_H

#include <string>

#include "agora_sig.h"
#include "receiverCallBack.h"
#include "senderCallBack.h"

using namespace agora_sdk_cpp;

// TEMPLATE CLASS:
// THIS IS BECAUSE THE API CALLBACKS ARE SIMILAR, BUT NOT SAME FROM SENDER AND RECEIVER
template < class callBack >
class lockAPI
{
    // PRIVATE DATA MEMBERS:
    private:
    
    // AGORA APPID AND THE SERIAL NUMBER
    std::string APPID;
    std::string LOCK_SERIAL;
    
    // STORES INSTANCE OF AGORA API AND CALLBACK OBJECT
    IAgoraAPI* msgAPI;
    callBack* msgCallBack;
    
    public:
    
    // ID: THE AGORA APPID
    lockAPI( std::string ID, std::string SERIAL );

    void login( std::string serial );
    
    bool getLoginFlag( );

    bool getSentFlag( );
    
    // SETS THE SENT_FLAG EQUAL TO FALSE
    void resetSentFlag( );
    
    // FUNCTION TO SEND SIGNAL OR MESSAGE
    //     to: SERIAL NUMBER OF RECEIVER DESIRED
    // signal: SIGNAL OR MESSAGE
    int sendSignal( const std::string &to, int encrypt, const std::string &signal );
    
    void logout( );
    
    bool getLogoutFlag( );
};

# endif
