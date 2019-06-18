// THIS IS THE MESSAGING CLASS, TO SEND MESSAGES AND ATTACHMENTS
var commHandler = function( )
{

    // TEMPORARY APPID FOR VIDEO CALLING
    var videoAPPID = "67f54a0cdad24b83a337baaede7cd29e";
    
    // MUTE AGORA API LOG ON CONSOLE
    AgoraRTC.Logger.setLogLevel( AgoraRTC.Logger.NONE );
    
    // THE VIDEO CLIENT OBJECT OF THE USER
    var client = null;
    
    // THE CHANNEL NAME OF THE CURRENT VIDEO CALL
    var channel = null;
    
    // THE LOCAL VIDEO STREAM OF THE USER
    var userStream = null;
    
    // VIDEO QUALITY OF CURRENT VIDEO CALL
    var localVideoProfile = "720P_3";
    
    // FUNCTION TO JOIN A CHANNEL FOR VIDEO CALLING
    // channelName: THE NAME OF THE CHANNEL ATTEMPTING TO JOIN
    //    password: THE PASSWORD FOR THE CHANNEL BEING JOINED
    this.joinVideoCall = function( channelName, password )
    {
        // OBTAIN CLIENT INSTANCE FROM AGORA
        client = AgoraRTC.createClient( { mode: "h264_interop" } );
        
        // FUNCTION FOR CLIENT INITIALIZATION FAILURE
        function onInitializeFail( ecode )
        {
            console.log( "CLIENT ERROR: " + ecode );
        };
        
        // INITIALIZE CLIENT OBJECT
        client.init( videoAPPID, null, onInitializeFail );
        
        // IF THE BROWSER HAS VIDEO SDK SUPPORT
        // IF THE CLIENT WAS INITIALIZED SUCCESFULLY
        // IF A USER IS CURRENTLY LOGGED IN
        if( AgoraRTC.checkSystemRequirements( ) && client )
        {
            // FUNCTION FOR SUCCESS IN JOINING CHANNEL
            function onJoinSuccess( uid )
            {
                // OUTPUT THE NAME OF CHANNEL JOINED, OUTPUT ITS UID
                console.log( "CHANNEL: " + channel + " | JOINED!");
                console.log( "UID: " + uid );
                
                // CREATE THE USER LOCAL VIDEO/AUDIO STREAM
                userStream = AgoraRTC.createStream( { streamID: uid, audio: true, video: true, screen: false } );
                
                // FUNCTION FOR SUCCESSFUL LOCAL VIDEO/AUDIO STREAM INITIALIZATION
                function onCreationSuccess( )
                {
                    // OUTPUT MESSAGE, OUTPUT THE STREAM UID
                    console.log( "LOCAL STREAM CREATION SUCCESSFUL!" );
                    console.log( "LOCAL STREAM ID: " + userStream.getId( ) );
                    
                    // DISPLAY LOCAL VIDEO STREAM
//                    userStream.enableVideo( );
//                    userStream.enableAudio( );
                    var video = document.getElementById( 'local_stream' );
                    userStream.play( video.id );
                    
                    // FUNCTION FOR SUCCESSFULLY PUBLISHING THE LOCAL
                    // VIDEO STREAM ON THE CHANNEL
                    function onPublicSuccess( event )
                    {
                        console.log( "LOCAL STREAM PUBLISHED!" );
                    };
                    
                    // FUNCTION FOR FAILURE IN PUBLISHING THE LOCAL
                    // VIDEO STREAM ON THE CHANNEL
                    function onPublishFail( ecode )
                    {
                        console.log( "LOCAL STREAM PUBLISH ERROR: " + ecode );
                    };
                    
                    // DESIGNATE CALLBACK FUNCTION ON SUCCESS
                    client.on( 'stream-published', onPublicSuccess );
                    
                    // PUBLISH LOCAL VIDEO STREAM ON CHANNEL
                    // DESIGNATE CALLBACK FOR FAILURE
                    client.publish( userStream, onPublishFail );
                };
                
                // FUNCTION FOR FAILURE IN LOCAL VIDEO/AUDIO STREAM INITIALIZATION
                function onCreationFail( ecode )
                {
                    console.log( "STREAM ERROR: " + ecode );
                };
                
                // INITIALIZE LOCAL VIDEO STREAM
                // DESIGNATE CALLBACKS FOR SUCCESS AND FAILURE
                userStream.init( onCreationSuccess, onCreationFail );
            };
            
            // FUNCTION FOR FAILURE IN JOINING CHANNEL
            function onJoinFailed( ecode )
            {
                console.log( "JOIN CHANNEL ERROR: " + ecode );
            };
            
            // STORE CHANNEL NAME
            channel = channelName;
            
            // SET ENCRYPTION LEVEL FOR SENDING VIDEO DATA TO AGORA
            client.setEncryptionMode( "aes-256-xts" );
            
            // SET CHANNEL PASSWORD
            client.setEncryptionSecret( password );
            
            // JOIN CHANNEL & DESIGNATE CALLBACKS FOR SUCCESS AND FAILURE
            client.join( null, channelName, null, onJoinSuccess, onJoinFailed );
            
            // FUNCTION FOR RECEIVING REMOTE VIDEO/AUDIO STREAMS
            function incomingStream( event )
            {
                // RETRIEVE THE REMOTE VIDEO/AUDIO STREAM
                remoteStream = event.stream;
                
                // OUTPUT ONCE STREAM IS RECEIVED AND ITS UID
                console.log( "INCOMING REMOTE STREAM!" );
                console.log( "REMOTE STREAM ID: " + remoteStream.getId( ) );
                
                // FUNCTION FOR FAILURE IN SUBSCRIBING TO REMOTE STREAM
                function onSubscriptionFailed( ecode )
                {
                    console.log( "REMOTE STREAM SUBSCRIPTION ERROR: " + ecode );
                };
                
                // SUBSCRIBE TO REMOTE AUDIO/VIDEO STREAM IN ORDER TO VIEW IT
                // AND DESIGNATE CALLBACK FOR FAILURE
                client.subscribe( remoteStream, onSubscriptionFailed );
            };
            
            // DESIGNATE CALLBACK FOR INCOMING VIDEO/AUDIO STREAM
            client.on( 'stream-added', incomingStream );
            
            // FUNCTION FOR SUCESSFUL SUBSCRIPTION TO REMOTE
            // AUDIO/VIDEO STREAM
            function onSubscriptionSuccess( event )
            {
                // RETRIEVE THE REMOTE VIDEO/AUDIO STREAM
                remoteStream = event.stream;
                
                // OUTPUT ONCE STREAM IS SUBSCRIBED TO
                console.log( "REMOTE STREAM SUBSCRIBED!" );
                
                // OBTAIN HTML ELEMENT FOR REMOTE VIDEO STREAM
                var video = document.getElementById( 'remote_stream' );
                
                // DISPLAY REMOTE VIDEO STREAM
                remoteStream.play( video.id );
            };
            
            // DESIGNATE CALLBACK FOR SUCCESSFUL SUBSCRIPTION TO
            // REMOTE AUDIO/VIDEO STREAM
            client.on( 'stream-subscribed', onSubscriptionSuccess );
        }
    };
};
