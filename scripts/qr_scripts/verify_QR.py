#!/usr/bin/env python3
import QR_functions as QRF
import sys
import time
import zmq

ZBAR_HEADER = 'QR-Code:'

context = zmq.Context()
socket  = context.socket( zmq.REQ )
socket.connect("tcp://localhost:5555")

ready_to_send = 1

verify_str    = 'unl'

from_zmq_str = ''

while(True):
    
    decode = sys.stdin.readline()
    decode = decode.strip()

    if decode.find( ZBAR_HEADER ) == 0:
        decode = decode [ len( ZBAR_HEADER ):  ]
        verified = QRF.Verify_QR(decode)
        if(verified == 1):
#            Send Verification Signal
            print("QR:VERIFIED, UNLOCKING...")
            if ready_to_send == 1:
                socket.send( ( ZBAR_HEADER + verify_str ).encode( 'utf-8' ) )
                ready_to_send = 0
            else:
                pass
        else:
            print("QR:NOT VERIFIED, REJECTING...")

    #wait for a response from zmq before sneding again
    if ready_to_send == 0:
        from_zmq_str = socket.recv()
        print ("QR:got response from zmq " + from_zmq_str.decode('utf-8') )
        ready_to_send = 1
    else:
        pass

    time.sleep( 1.000 )