
#ifndef _AGORA_SDK_CPP_H
#define _AGORA_SDK_CPP_H

#define LIB_PRE
#ifdef __cplusplus
#include <string>

#if defined(_WIN32)
#include <cstdint>
#endif

namespace agora_sdk_cpp
{
    typedef enum {
        SUCCESS = 0,
        LOGOUT_E_OTHER = 100,
        LOGOUT_E_USER = 101,
        LOGOUT_E_NET = 102,
        LOGOUT_E_KICKED = 103,
        LOGOUT_E_PACKET = 104,
        LOGOUT_E_TOKENEXPIRED = 105,
        LOGOUT_E_OLDVERSION = 106,
        LOGOUT_E_TOKENWRONG = 107,
        LOGOUT_E_ALREADY_LOGOUT = 108,
        LOGIN_E_OTHER = 200,
        LOGIN_E_NET = 201,
        LOGIN_E_FAILED = 202,
        LOGIN_E_CANCEL = 203,
        LOGIN_E_TOKENEXPIRED = 204,
        LOGIN_E_OLDVERSION = 205,
        LOGIN_E_TOKENWRONG = 206,
        LOGIN_E_TOKEN_KICKED = 207,
        LOGIN_E_ALREADY_LOGIN = 208,
        LOGIN_E_INVALID_USER = 209,
        JOINCHANNEL_E_OTHER = 300,
        SENDMESSAGE_E_OTHER = 400,
        SENDMESSAGE_E_TIMEOUT = 401,
        QUERYUSERNUM_E_OTHER = 500,
        QUERYUSERNUM_E_TIMEOUT = 501,
        QUERYUSERNUM_E_BYUSER = 502,
        LEAVECHANNEL_E_OTHER = 600,
        LEAVECHANNEL_E_KICKED = 601,
        LEAVECHANNEL_E_BYUSER = 602,
        LEAVECHANNEL_E_LOGOUT = 603,
        LEAVECHANNEL_E_DISCONN = 604,
        INVITE_E_OTHER = 700,
        INVITE_E_REINVITE = 701,
        INVITE_E_NET = 702,
        INVITE_E_PEEROFFLINE = 703,
        INVITE_E_TIMEOUT = 704,
        INVITE_E_CANTRECV = 705,
        GENERAL_E = 1000,
        GENERAL_E_FAILED = 1001,
        GENERAL_E_UNKNOWN = 1002,
        GENERAL_E_NOT_LOGIN = 1003,
        GENERAL_E_WRONG_PARAM = 1004,
        GENERAL_E_LARGE_PARAM = 1005
    }AGORA_E_CODE;

  class ICallBack
  {
  public:
    virtual void onReconnecting(uint32_t nretry) {}
    virtual void onReconnected(int fd) {}
    virtual void onLoginSuccess(uint32_t uid,int fd) {}
    virtual void onLogout(int ecode) {}
    virtual void onLoginFailed(int ecode) {}
    virtual void onChannelJoined(char const * channelID, size_t channelID_size) {}
    virtual void onChannelJoinFailed(char const * channelID, size_t channelID_size,int ecode) {}
    virtual void onChannelLeaved(char const * channelID, size_t channelID_size,int ecode) {}
    virtual void onChannelUserJoined(char const * account, size_t account_size,uint32_t uid) {}
    virtual void onChannelUserLeaved(char const * account, size_t account_size,uint32_t uid) {}
    virtual void onChannelUserList(int n,char** accounts,uint32_t* uids) {}
    virtual void onChannelQueryUserNumResult(char const * channelID, size_t channelID_size,int ecode,int num) {}
    virtual void onChannelQueryUserIsIn(char const * channelID, size_t channelID_size,char const * account, size_t account_size,int isIn) {}
    virtual void onChannelAttrUpdated(char const * channelID, size_t channelID_size,char const * name, size_t name_size,char const * value, size_t value_size,char const * type, size_t type_size) {}
    virtual void onInviteReceived(char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid,char const * extra, size_t extra_size) {}
    virtual void onInviteReceivedByPeer(char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid) {}
    virtual void onInviteAcceptedByPeer(char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid,char const * extra, size_t extra_size) {}
    virtual void onInviteRefusedByPeer(char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid,char const * extra, size_t extra_size) {}
    virtual void onInviteFailed(char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid,int ecode,char const * extra, size_t extra_size) {}
    virtual void onInviteEndByPeer(char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid,char const * extra, size_t extra_size) {}
    virtual void onInviteEndByMyself(char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid) {}
    virtual void onInviteMsg(char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid,char const * msgType, size_t msgType_size,char const * msgData, size_t msgData_size,char const * extra, size_t extra_size) {}
    virtual void onMessageSendError(char const * messageID, size_t messageID_size,int ecode) {}
    virtual void onMessageSendProgress(char const * account, size_t account_size,char const * messageID, size_t messageID_size,char const * type, size_t type_size,char const * info, size_t info_size) {}
    virtual void onMessageSendSuccess(char const * messageID, size_t messageID_size) {}
    virtual void onMessageAppReceived(char const * msg, size_t msg_size) {}
    virtual void onMessageInstantReceive(char const * account, size_t account_size,uint32_t uid,char const * msg, size_t msg_size) {}
    virtual void onMessageChannelReceive(char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid,char const * msg, size_t msg_size) {}
    virtual void onLog(char const * txt, size_t txt_size) {}
    virtual void onInvokeRet(char const * callID, size_t callID_size,char const * err, size_t err_size,char const * resp, size_t resp_size) {}
    virtual void onMsg(char const * from, size_t from_size,char const * t, size_t t_size,char const * msg, size_t msg_size) {}
    virtual void onUserAttrResult(char const * account, size_t account_size,char const * name, size_t name_size,char const * value, size_t value_size) {}
    virtual void onUserAttrAllResult(char const * account, size_t account_size,char const * value, size_t value_size) {}
    virtual void onError(char const * name, size_t name_size,int ecode,char const * desc, size_t desc_size) {}
    virtual void onQueryUserStatusResult(char const * name, size_t name_size,char const * status, size_t status_size) {}
    virtual void onDbg(char const * a, size_t a_size,char const * b, size_t b_size) {}
    virtual void onBCCall_result(char const * reason, size_t reason_size,char const * json_ret, size_t json_ret_size,char const * callID, size_t callID_size) {}
  };

  class IAgoraAPI
  {
  public:
    virtual void callbackSet (ICallBack* handler) = 0;
    virtual ICallBack* callbackGet () = 0;
    
    
    virtual void login (char const * vendorID, size_t vendorID_size,char const * account, size_t account_size,char const * token, size_t token_size,uint32_t uid,char const * deviceID, size_t deviceID_size) = 0;
    virtual void login2 (char const * vendorID, size_t vendorID_size,char const * account, size_t account_size,char const * token, size_t token_size,uint32_t uid,char const * deviceID, size_t deviceID_size,int retry_time_in_s,int retry_count) = 0;
    virtual void logout () = 0;
    virtual void channelJoin (char const * channelID, size_t channelID_size) = 0;
    virtual void channelLeave (char const * channelID, size_t channelID_size) = 0;
    virtual void channelQueryUserNum (char const * channelID, size_t channelID_size) = 0;
    virtual void channelQueryUserIsIn (char const * channelID, size_t channelID_size,char const * account, size_t account_size) = 0;
    virtual void channelSetAttr (char const * channelID, size_t channelID_size,char const * name, size_t name_size,char const * value, size_t value_size) = 0;
    virtual void channelDelAttr (char const * channelID, size_t channelID_size,char const * name, size_t name_size) = 0;
    virtual void channelClearAttr (char const * channelID, size_t channelID_size) = 0;
    virtual void channelInviteUser (char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid=0) = 0;
    virtual void channelInviteUser2 (char const * channelID, size_t channelID_size,char const * account, size_t account_size,char const * extra, size_t extra_size) = 0;
    virtual void channelInvitePhone (char const * channelID, size_t channelID_size,char const * phoneNum, size_t phoneNum_size,uint32_t uid=0) = 0;
    virtual void channelInvitePhone2 (char const * channelID, size_t channelID_size,char const * phoneNum, size_t phoneNum_size,char const * sourcesNum, size_t sourcesNum_size) = 0;
    virtual void channelInvitePhone3 (char const * channelID, size_t channelID_size,char const * phoneNum, size_t phoneNum_size,char const * sourcesNum, size_t sourcesNum_size,char const * extra, size_t extra_size) = 0;
    virtual void channelInviteDTMF (char const * channelID, size_t channelID_size,char const * phoneNum, size_t phoneNum_size,char const * dtmf, size_t dtmf_size) = 0;
    virtual void channelInviteAccept (char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid,char const * extra, size_t extra_size) = 0;
    virtual void channelInviteRefuse (char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid,char const * extra, size_t extra_size) = 0;
    virtual void channelInviteEnd (char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid) = 0;
    virtual void messageAppSend (char const * msg, size_t msg_size,char const * msgID, size_t msgID_size) = 0;
    virtual void messageInstantSend (char const * account, size_t account_size,uint32_t uid,char const * msg, size_t msg_size,char const * msgID, size_t msgID_size) = 0;
    virtual void messageInstantSend2 (char const * account, size_t account_size,uint32_t uid,char const * msg, size_t msg_size,char const * msgID, size_t msgID_size,char const * options, size_t options_size) = 0;
    virtual void messageChannelSend (char const * channelID, size_t channelID_size,char const * msg, size_t msg_size,char const * msgID, size_t msgID_size) = 0;
    virtual void messageChannelSendForce (char const * channelID, size_t channelID_size,char const * msg, size_t msg_size,char const * msgID, size_t msgID_size) = 0;
    virtual void messageChannelSend2 (char const * channelID, size_t channelID_size,char const * msg, size_t msg_size,char const * msgID, size_t msgID_size,char const * extra, size_t extra_size) = 0;
    virtual void messagePushSend (char const * account, size_t account_size,uint32_t uid,char const * msg, size_t msg_size,char const * msgID, size_t msgID_size) = 0;
    virtual void messageChatSend (char const * account, size_t account_size,uint32_t uid,char const * msg, size_t msg_size,char const * msgID, size_t msgID_size) = 0;
    virtual void messageDTMFSend (uint32_t uid,char const * msg, size_t msg_size,char const * msgID, size_t msgID_size) = 0;
    virtual void setBackground (uint32_t bOut) = 0;
    virtual void setNetworkStatus (uint32_t bOut) = 0;
    virtual void ping () = 0;
    virtual void setAttr (char const * name, size_t name_size,char const * value, size_t value_size) = 0;
    virtual void getAttr (char const * name, size_t name_size) = 0;
    virtual void getAttrAll () = 0;
    virtual void getUserAttr (char const * account, size_t account_size,char const * name, size_t name_size) = 0;
    virtual void getUserAttrAll (char const * account, size_t account_size) = 0;
    virtual void queryUserStatus (char const * account, size_t account_size) = 0;
    virtual void invoke (char const * name, size_t name_size,char const * req, size_t req_size,char const * callID, size_t callID_size) = 0;
    virtual void start () = 0;
    virtual void stop () = 0;
    virtual bool isOnline () = 0;
    virtual int getStatus () = 0;
    virtual int getSdkVersion () = 0;
    virtual void bc_call (char const * func, size_t func_size,char const * json_args, size_t json_args_size,char const * callID, size_t callID_size) = 0;
    virtual void dbg (char const * a, size_t a_size,char const * b, size_t b_size) = 0;
    virtual void destroy () = 0;
  };
}

//#define LIB_PRE __declspec(dllexport)
#define LIB_PRE
extern "C" LIB_PRE agora_sdk_cpp::IAgoraAPI* getAgoraSDKInstanceCPP();
extern "C" LIB_PRE agora_sdk_cpp::IAgoraAPI* createAgoraSDKInstanceCPP();
#endif

#ifndef __cplusplus
typedef unsigned int size_t;
typedef unsigned int uint32_t;
typedef int bool;
#endif

typedef struct ICallBackC{
    void (*onReconnecting)(struct ICallBackC* self,uint32_t nretry);
    void (*onReconnected)(struct ICallBackC* self,int fd);
    void (*onLoginSuccess)(struct ICallBackC* self,uint32_t uid,int fd);
    void (*onLogout)(struct ICallBackC* self,int ecode);
    void (*onLoginFailed)(struct ICallBackC* self,int ecode);
    void (*onChannelJoined)(struct ICallBackC* self,char const * channelID, size_t channelID_size);
    void (*onChannelJoinFailed)(struct ICallBackC* self,char const * channelID, size_t channelID_size,int ecode);
    void (*onChannelLeaved)(struct ICallBackC* self,char const * channelID, size_t channelID_size,int ecode);
    void (*onChannelUserJoined)(struct ICallBackC* self,char const * account, size_t account_size,uint32_t uid);
    void (*onChannelUserLeaved)(struct ICallBackC* self,char const * account, size_t account_size,uint32_t uid);
    void (*onChannelUserList)(struct ICallBackC* self,int n,char** accounts,uint32_t* uids);
    void (*onChannelQueryUserNumResult)(struct ICallBackC* self,char const * channelID, size_t channelID_size,int ecode,int num);
    void (*onChannelQueryUserIsIn)(struct ICallBackC* self,char const * channelID, size_t channelID_size,char const * account, size_t account_size,int isIn);
    void (*onChannelAttrUpdated)(struct ICallBackC* self,char const * channelID, size_t channelID_size,char const * name, size_t name_size,char const * value, size_t value_size,char const * type, size_t type_size);
    void (*onInviteReceived)(struct ICallBackC* self,char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid,char const * extra, size_t extra_size);
    void (*onInviteReceivedByPeer)(struct ICallBackC* self,char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid);
    void (*onInviteAcceptedByPeer)(struct ICallBackC* self,char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid,char const * extra, size_t extra_size);
    void (*onInviteRefusedByPeer)(struct ICallBackC* self,char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid,char const * extra, size_t extra_size);
    void (*onInviteFailed)(struct ICallBackC* self,char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid,int ecode,char const * extra, size_t extra_size);
    void (*onInviteEndByPeer)(struct ICallBackC* self,char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid,char const * extra, size_t extra_size);
    void (*onInviteEndByMyself)(struct ICallBackC* self,char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid);
    void (*onInviteMsg)(struct ICallBackC* self,char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid,char const * msgType, size_t msgType_size,char const * msgData, size_t msgData_size,char const * extra, size_t extra_size);
    void (*onMessageSendError)(struct ICallBackC* self,char const * messageID, size_t messageID_size,int ecode);
    void (*onMessageSendProgress)(struct ICallBackC* self,char const * account, size_t account_size,char const * messageID, size_t messageID_size,char const * type, size_t type_size,char const * info, size_t info_size);
    void (*onMessageSendSuccess)(struct ICallBackC* self,char const * messageID, size_t messageID_size);
    void (*onMessageAppReceived)(struct ICallBackC* self,char const * msg, size_t msg_size);
    void (*onMessageInstantReceive)(struct ICallBackC* self,char const * account, size_t account_size,uint32_t uid,char const * msg, size_t msg_size);
    void (*onMessageChannelReceive)(struct ICallBackC* self,char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid,char const * msg, size_t msg_size);
    void (*onLog)(struct ICallBackC* self,char const * txt, size_t txt_size);
    void (*onInvokeRet)(struct ICallBackC* self,char const * callID, size_t callID_size,char const * err, size_t err_size,char const * resp, size_t resp_size);
    void (*onMsg)(struct ICallBackC* self,char const * from, size_t from_size,char const * t, size_t t_size,char const * msg, size_t msg_size);
    void (*onUserAttrResult)(struct ICallBackC* self,char const * account, size_t account_size,char const * name, size_t name_size,char const * value, size_t value_size);
    void (*onUserAttrAllResult)(struct ICallBackC* self,char const * account, size_t account_size,char const * value, size_t value_size);
    void (*onError)(struct ICallBackC* self,char const * name, size_t name_size,int ecode,char const * desc, size_t desc_size);
    void (*onQueryUserStatusResult)(struct ICallBackC* self,char const * name, size_t name_size,char const * status, size_t status_size);
    void (*onDbg)(struct ICallBackC* self,char const * a, size_t a_size,char const * b, size_t b_size);
    void (*onBCCall_result)(struct ICallBackC* self,char const * reason, size_t reason_size,char const * json_ret, size_t json_ret_size,char const * callID, size_t callID_size);
}ICallBackC;

typedef struct IAgoraAPIC{
    void (*callbackSet)(struct IAgoraAPIC* self,ICallBackC* handler);
    ICallBackC* (*callbackGet)(struct IAgoraAPIC* self);
    
    
    void (*login)(struct IAgoraAPIC* self,char const * vendorID, size_t vendorID_size,char const * account, size_t account_size,char const * token, size_t token_size,uint32_t uid,char const * deviceID, size_t deviceID_size);
    void (*login2)(struct IAgoraAPIC* self,char const * vendorID, size_t vendorID_size,char const * account, size_t account_size,char const * token, size_t token_size,uint32_t uid,char const * deviceID, size_t deviceID_size,int retry_time_in_s,int retry_count);
    void (*logout)(struct IAgoraAPIC* self);
    void (*channelJoin)(struct IAgoraAPIC* self,char const * channelID, size_t channelID_size);
    void (*channelLeave)(struct IAgoraAPIC* self,char const * channelID, size_t channelID_size);
    void (*channelQueryUserNum)(struct IAgoraAPIC* self,char const * channelID, size_t channelID_size);
    void (*channelQueryUserIsIn)(struct IAgoraAPIC* self,char const * channelID, size_t channelID_size,char const * account, size_t account_size);
    void (*channelSetAttr)(struct IAgoraAPIC* self,char const * channelID, size_t channelID_size,char const * name, size_t name_size,char const * value, size_t value_size);
    void (*channelDelAttr)(struct IAgoraAPIC* self,char const * channelID, size_t channelID_size,char const * name, size_t name_size);
    void (*channelClearAttr)(struct IAgoraAPIC* self,char const * channelID, size_t channelID_size);
    void (*channelInviteUser)(struct IAgoraAPIC* self,char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid);
    void (*channelInviteUser2)(struct IAgoraAPIC* self,char const * channelID, size_t channelID_size,char const * account, size_t account_size,char const * extra, size_t extra_size);
    void (*channelInvitePhone)(struct IAgoraAPIC* self,char const * channelID, size_t channelID_size,char const * phoneNum, size_t phoneNum_size,uint32_t uid);
    void (*channelInvitePhone2)(struct IAgoraAPIC* self,char const * channelID, size_t channelID_size,char const * phoneNum, size_t phoneNum_size,char const * sourcesNum, size_t sourcesNum_size);
    void (*channelInvitePhone3)(struct IAgoraAPIC* self,char const * channelID, size_t channelID_size,char const * phoneNum, size_t phoneNum_size,char const * sourcesNum, size_t sourcesNum_size,char const * extra, size_t extra_size);
    void (*channelInviteDTMF)(struct IAgoraAPIC* self,char const * channelID, size_t channelID_size,char const * phoneNum, size_t phoneNum_size,char const * dtmf, size_t dtmf_size);
    void (*channelInviteAccept)(struct IAgoraAPIC* self,char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid,char const * extra, size_t extra_size);
    void (*channelInviteRefuse)(struct IAgoraAPIC* self,char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid,char const * extra, size_t extra_size);
    void (*channelInviteEnd)(struct IAgoraAPIC* self,char const * channelID, size_t channelID_size,char const * account, size_t account_size,uint32_t uid);
    void (*messageAppSend)(struct IAgoraAPIC* self,char const * msg, size_t msg_size,char const * msgID, size_t msgID_size);
    void (*messageInstantSend)(struct IAgoraAPIC* self,char const * account, size_t account_size,uint32_t uid,char const * msg, size_t msg_size,char const * msgID, size_t msgID_size);
    void (*messageInstantSend2)(struct IAgoraAPIC* self,char const * account, size_t account_size,uint32_t uid,char const * msg, size_t msg_size,char const * msgID, size_t msgID_size,char const * options, size_t options_size);
    void (*messageChannelSend)(struct IAgoraAPIC* self,char const * channelID, size_t channelID_size,char const * msg, size_t msg_size,char const * msgID, size_t msgID_size);
    void (*messageChannelSendForce)(struct IAgoraAPIC* self,char const * channelID, size_t channelID_size,char const * msg, size_t msg_size,char const * msgID, size_t msgID_size);
    void (*messageChannelSend2)(struct IAgoraAPIC* self,char const * channelID, size_t channelID_size,char const * msg, size_t msg_size,char const * msgID, size_t msgID_size,char const * extra, size_t extra_size);
    void (*messagePushSend)(struct IAgoraAPIC* self,char const * account, size_t account_size,uint32_t uid,char const * msg, size_t msg_size,char const * msgID, size_t msgID_size);
    void (*messageChatSend)(struct IAgoraAPIC* self,char const * account, size_t account_size,uint32_t uid,char const * msg, size_t msg_size,char const * msgID, size_t msgID_size);
    void (*messageDTMFSend)(struct IAgoraAPIC* self,uint32_t uid,char const * msg, size_t msg_size,char const * msgID, size_t msgID_size);
    void (*setBackground)(struct IAgoraAPIC* self,uint32_t bOut);
    void (*setNetworkStatus)(struct IAgoraAPIC* self,uint32_t bOut);
    void (*ping)(struct IAgoraAPIC* self);
    void (*setAttr)(struct IAgoraAPIC* self,char const * name, size_t name_size,char const * value, size_t value_size);
    void (*getAttr)(struct IAgoraAPIC* self,char const * name, size_t name_size);
    void (*getAttrAll)(struct IAgoraAPIC* self);
    void (*getUserAttr)(struct IAgoraAPIC* self,char const * account, size_t account_size,char const * name, size_t name_size);
    void (*getUserAttrAll)(struct IAgoraAPIC* self,char const * account, size_t account_size);
    void (*queryUserStatus)(struct IAgoraAPIC* self,char const * account, size_t account_size);
    void (*invoke)(struct IAgoraAPIC* self,char const * name, size_t name_size,char const * req, size_t req_size,char const * callID, size_t callID_size);
    void (*start)(struct IAgoraAPIC* self);
    void (*stop)(struct IAgoraAPIC* self);
    bool (*isOnline)(struct IAgoraAPIC* self);
    int (*getStatus)(struct IAgoraAPIC* self);
    int (*getSdkVersion)(struct IAgoraAPIC* self);
    void (*bc_call)(struct IAgoraAPIC* self,char const * func, size_t func_size,char const * json_args, size_t json_args_size,char const * callID, size_t callID_size);
    void (*dbg)(struct IAgoraAPIC* self,char const * a, size_t a_size,char const * b, size_t b_size);
    void (*destroy)(struct IAgoraAPIC* self);
    void (*release)(struct IAgoraAPIC* self);
    void *api;
    void *cb;
}IAgoraAPIC;

#ifdef __cplusplus
extern "C" IAgoraAPIC* getAgoraSDKInstanceC();
extern "C" IAgoraAPIC* createAgoraSDKInstanceC();
#else
IAgoraAPIC* getAgoraSDKInstanceC();
IAgoraAPIC* createAgoraSDKInstanceC();
#endif

#endif

