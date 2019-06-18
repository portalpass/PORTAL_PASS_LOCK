#!/usr/bin/env python3

#needs package pyserial. sudo pip install pyserial
#Uses zmq to communicate with lockReceiver
#Will run commands as directed by the receiver and can send info back
#Requires lockSender to be installed via sudo -E make install, so it is on
#the path

import os
import serial
import time
import zmq

CLEAR_CHARACTER = b'*'
TERMINATING_CHARACTER = b'X'
SPONT_HEADER = 'SPONT:'
REPLY_HEADER = 'REPLY:'
ZBAR_HEADER  = 'QR-Code:'

#initialize with port name.
#In UNIX /dev/tty.usb*, OR /dev/ACM* OR /dev/ttyUSB*
#In Windows COMX

ser = serial.Serial( port='/dev/ttyUSB0', baudrate=9600, bytesize=serial.EIGHTBITS, 
                     parity=serial.PARITY_NONE, timeout=None, write_timeout=None )

context = zmq.Context()
socket  = context.socket(zmq.REP)
socket.bind("tcp://*:5555")

from_arduino = ''

send_zmq_str = ''
send_lockSender_str = ''
tmp_str = ''

qr_verify                = 0
ready_to_send_zmq        = 0
is_zmq_data              = 0
ready_to_send_lockSender = 0

while True:
    #if there are bytes to read, read them
    if ( ser.in_waiting > 0 ):
        from_arduino += ser.read(ser.inWaiting()).decode('ascii')
    
    #if terminating character is found, capture everything
    #up until then, process, then chop it off from that point
    find_pos = from_arduino.find( TERMINATING_CHARACTER.decode('ascii') )
   
    #if arduino attached the SPONT_HEADER, then we know lockReceiver
    #didnt request it . Send it back with lockSender
    #Otherwise, we did request it, so give it back to lockReceiver over ZMQ
    #If the arduino replies from a QR verification, the QR will need a ZMQ response
    #and the sender will also need a reply
    is_zmq_data  = 0
    if find_pos != -1:
        tmp_str      = from_arduino[ 0 : find_pos ]
        print('LC:received message from arduino! ' + tmp_str )
        from_arduino = from_arduino[ find_pos + 1 : ]
        
        if ( tmp_str.find( SPONT_HEADER ) == 0 ):
            ready_to_send_lockSender = 1
            send_lockSender_str = tmp_str[ len( SPONT_HEADER ) : ]
        
        elif tmp_str.find( REPLY_HEADER ) == 0:
            is_zmq_data = 1
            send_zmq_str = tmp_str[ len( REPLY_HEADER ) : ]
            if qr_verify == 1:
                ready_to_send_lockSender = 1
                send_lockSender_str = send_zmq_str
                qr_verify = 0
    
    #means this message came as a request from receiver, reply
    #to lockReceiver
    if ready_to_send_zmq == 1 and is_zmq_data == 1:
        if len ( send_zmq_str ) == 0: 
            send_zmq_str = 'OK'
        print ( "LC:sending response to requester \"" + send_zmq_str +"\"" )
        socket.send( send_zmq_str.encode( 'utf-8' ) )
        ready_to_send_zmq = 0
        send_zmq_str = ''
   
    #means this message was asynchronous, send via 
    #lockSender
    if ready_to_send_lockSender == 1:
        print ( "LC:sending using lock sender \"" + send_lockSender_str + "\"" )
        os.system( 'lockSender \"' + send_lockSender_str +'\"')
        ready_to_send_lockSender = 0
        send_lockSender_str = ''

    #try to read in a message in ZMQ
    #forward it to arduino
    #if it came from QR, mark it so the answer is sent to both
    #lockSender and verify_QR
    if ready_to_send_zmq == 0:
        try:
            from_zmq_str = socket.recv(flags=zmq.NOBLOCK)
            print ( "LC:Received request: %s" % from_zmq_str )
            if ZBAR_HEADER in from_zmq_str.decode('utf-8'):
                qr_verify = 1
                from_zmq_str = from_zmq_str [ len( ZBAR_HEADER ) : ]
            ready_to_send_zmq = 1 
            from_zmq_str = CLEAR_CHARACTER + from_zmq_str + TERMINATING_CHARACTER
            ser.write( from_zmq_str )
        except zmq.Again as e:
            pass
    
    time.sleep( 1.000 )
