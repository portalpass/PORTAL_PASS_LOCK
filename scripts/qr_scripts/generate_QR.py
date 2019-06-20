#!/usr/bin/env python3

# Takes in list of conditions and creates a 6 element list
# Generates a QRID using random numebr generator
# Logs hashed QRID and list of conditions into the database qr.txt 

import QR_functions as QRF
import sys

# Expects 6 arguments 
if( (len(sys.argv)-1) == 6):
    cond = ['','','','','','']
    for i in range(6):
        cond[i] = str(sys.argv[i+1])
    QRF.Generate_QR(cond)