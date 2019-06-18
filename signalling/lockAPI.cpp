//implementation file of lockAPI.h
#include <iostream>

#include "lockAPI.h"
#include "helper_functions.h"

using namespace agora_sdk_cpp;

static const std::string sender_Header    = "S";
static const std::string receiver_Header  = "R";
static const std::string android_Header   = "A";
static const std::string encrypt_Header   = "E";
static const std::string unencrypt_Header = "U";

static const std::string python_dir = helper::get_python_dir();
static const std::string conf_dir   = helper::get_conf_dir();

//only these templates can be used, this must be edited to expand template list
template class lockAPI<receiverCallBack>;
template class lockAPI<senderCallBack>;

template < class callBack >
lockAPI< callBack >::lockAPI( std::string ID, std::string SERIAL )
{
    // STORE THE APPID & serial
    APPID = ID;
    LOCK_SERIAL = SERIAL;

    // GET AGORA API INSTANCE
    msgAPI = getAgoraSDKInstanceCPP( );
    
    // CREATE CALLBACK OBJECT, PASS AGORA API INSTANCE TO CALLBACK
    // DOING SO ALLOWS THE CALLBACKS TO SEND MESSAGES
    msgCallBack = new callBack( msgAPI );
    
    // PASS CALLBACK OBJECT TO AGORA API INSTANCE
    msgAPI->callbackSet( msgCallBack );
}

template < class callBack >
void lockAPI< callBack >::login( std::string serial )
{
    // IF AN AGORA API INSTANCE WAS CREATED
    if( msgAPI )
    {
        // PASS THE LOCK SERIAL TO CALLBACK
        msgCallBack->setSerial( serial );
        
        // LOG INTO THE AGORA COMM SYSTEM
        std::string token = "_no_need_token";
        msgAPI->login( APPID.data(), APPID.size(), serial.data(), serial.size(), token.data(), token.size(), 0, NULL, 0 );
    }
}

template < class callBack >
bool lockAPI< callBack >::getLoginFlag()
{
    return msgCallBack->getLoginFlag( );
}

template < class callBack >
bool lockAPI< callBack >::getSentFlag()
{
    return msgCallBack->getSentFlag( );
}

template < class callBack >
void lockAPI< callBack >::resetSentFlag( )
{
    msgCallBack->resetSentFlag( );
}

//send a message
//If encrypt flag is set, encrypt, then set to base64
template < class callBack >
int lockAPI< callBack >::sendSignal( const std::string &to, int encrypt, const std::string &signal )
{
    // IF RECEIVER OR SENDER SUCCESSFULLY LOGGED IN: true

    std::string signal_out;

    if( getLoginFlag( ) )
    {
    
        if ( encrypt != 0 )
        {
            std::string  pubk_f  = conf_dir + "/" + to + ".pub.pem";
            std::string  privk_f = conf_dir + "/" + LOCK_SERIAL + ".priv.pem";

            if ( helper::rsa_encrypt_sign( signal, pubk_f, privk_f, signal_out ) )
            {
                std::cerr << "Error encrypting" << std::endl;
                return 3;
            }
            
            signal_out = helper::base64_encode( reinterpret_cast<const unsigned char*>( signal_out.data() ), signal_out.size() );
            signal_out = encrypt_Header + signal_out;
        }
        
        else
        {
            signal_out = signal;
            signal_out = unencrypt_Header + signal_out;
        }

        signal_out = sender_Header + signal_out;

        msgAPI->messageInstantSend( to.data(), to.size(), 0, signal_out.data(), signal_out.size(), NULL, 0 );
        return 0;
    }

    else
    {
        std::cerr <<"ERROR LOGGING IN" << std::endl;
        return 1;
    }
}

template < class callBack >
void lockAPI< callBack >::logout()
{
    // IF RECEIVER OR SENDER SUCCESSFULLY LOGGED IN: true
    if( getLoginFlag( ) )
    {
        // AGORA API LOGOUT
        msgAPI->logout();
    }
}

template < class callBack >
bool lockAPI < callBack >::getLogoutFlag( )
{
    return msgCallBack->getLogoutFlag( );
}

