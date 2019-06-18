#!/usr/bin/env python3

import bcrypt
import datetime
import base64
import io
import os
import qrcode
import random

#import lock_control_functions as LCF

QR = {}
database_path = os.path.expanduser('~')+'/.config/PORTAL_PASS/qr.txt'


#Send Hash Table Info to txt file
def Update_File():
    global QR
    now = datetime.datetime.now()    
    with open(database_path,'w') as FILE:
        FILE.seek(0)
        FILE.truncate()
        for i in QR:
            
            cond = QR[i]
            eyear = emonth = eday = ''
            dash = 0
            for char in cond[1]:
                if(char == '-'):
                    dash += 1
                else:
                    if(dash == 0):
                        eyear += char
                    if(dash == 1):
                        emonth += char
                    if(dash == 2):
                        eday += char
            eyear = int(eyear)
            emonth = int(emonth)
            eday = int(eday)
            
            times_up = 0
            if(now.year>eyear):
                times_up = 1
            if(now.year==eyear and now.month>emonth):
                times_up = 1
            if(now.year==eyear and now.month==emonth and now.day>eday):
                times_up = 1
            
            if(times_up == 0):
                if(int(cond[5])==-1 or int(cond[5])>0):
                    line = ''
                    line += str(i) + ' '
                    for j in range(6):
                        if(j<5):
                            line += str(QR[i][j]) + ' '
                        if(j==5):    
                            line += str(QR[i][j]) + '\n'
                    FILE.write(line)
                    FILE.flush()
            
#Send txt file info to Hash Table
def Update_Dict():
    global QR
    with open(database_path,'r') as FILE:
        lines = len(FILE.readlines())
        FILE.seek(0)
        for i in range(lines):
            line = FILE.readline()
            space = 0
            qrid = ''
            cond = ['','','','','','']
            for char in line:
                if(char == ' '):
                    space += 1
                else:
                    if(space == 0):
                        qrid += char
                    for i in range(6):
                        if(space == i+1):
                            cond[i] += char
                if(cond[5].endswith('\n')):
                    cond[5] = cond[5][:-1]
            QR[qrid] = cond
            
#-----------------------------------------------------

def Generate_QR(cond):
    Update_Dict()
    
    is_repeat = 0
    for i in QR:
        if(QR[i]==cond):
            is_repeat = 1
    if(is_repeat==1):
#        print('Repeat. Not Updating Database. :(')
        pass
    
    is_valid = 0
    is_valid = Check_Valid_Time(cond)
    if(is_valid==0):
#        print('Invalid Conditions')
        pass
    
    if(is_repeat == 0 and is_valid==1):
#    Generate random qrid
        random.seed(None)
        qr_length = 100
        qrid = ''
        for i in range(qr_length):
            qrid += chr(random.randint(0x30,0x7e))

        qr = qrcode.QRCode(
                version=1,
                error_correction=qrcode.constants.ERROR_CORRECT_H,
                box_size=10,
                border=4)
        qr.add_data(qrid)
        qr.make(fit=True)
    
        img = qr.make_image(fill_color="black", back_color="white")
        img    = qrcode.make(qrid)
        imgRaw = io.BytesIO()
        img.save(imgRaw, format='PNG')
    
        img64 = base64.b64encode(imgRaw.getvalue())
        img64 = img64.decode("ASCII")
        img64 = img64.strip()
        print (img64,end="")
    
#     Hash qrid and store in database 
        hqrid = bcrypt.hashpw(qrid.encode('utf8'),bcrypt.gensalt())
        hqrid = hqrid.decode('utf8')
        QR[hqrid] = cond
        Update_File()
    
#------------------------------------------------------
def Check_Valid_Time(cond):
    now = datetime.datetime.now()
    syear = smonth = sday = eyear = emonth = eday = ''
    shour = sminute = ehour = eminute = ''
    
    dash = 0
    for char in cond[0]:
        if(char == '-'):
            dash += 1
        else:
            if(dash == 0):
                syear += char
            if(dash == 1):
                smonth += char
            if(dash == 2):
                sday += char
    syear = int(syear)
    smonth = int(smonth)
    sday = int(sday)
    
        
    dash = 0
    for char in cond[1]:
        if(char == '-'):
            dash += 1
        else:
            if(dash == 0):
                eyear += char
            if(dash == 1):
                emonth += char
            if(dash == 2):
                eday += char
    eyear = int(eyear)
    emonth = int(emonth)
    eday = int(eday)
        
    dash = 0
    for char in cond[2]:
        if(char == '-'):
            dash += 1
        else:
            if(dash == 0):
                shour += char
            if(dash == 1):
                sminute += char
    shour = int(shour)
    sminute = int(sminute)
        
    dash = 0
    for char in cond[3]:
        if(char == '-'):
            dash += 1
        else:
            if(dash == 0):
                ehour += char
            if(dash == 1):
                eminute += char
    ehour = int(ehour)
    eminute = int(eminute)
    
    dow = int(cond[4],2)
    uses_left = int(cond[5])
    
    valid_year = 0
    if(syear>=now.year and eyear>=syear):
        valid_year = 1
    
    valid_month = 0        
    if(eyear>syear):
        if(smonth<=12 and smonth>=1 and emonth<=12 and emonth>=1):
            valid_month = 1
    elif(eyear==syear):
        if(emonth>=smonth):
            valid_month = 1
        
    valid_day = 0
    valid_sday = 0
    valid_eday = 0
    if(smonth==1 or smonth==3 or smonth==5 or smonth==7 or smonth==8 or smonth==10 or smonth==12):
        if(sday>=1 and sday<=31):
            valid_sday = 1
    elif(smonth==4 or smonth==6 or smonth==9 or smonth==11):
        if(sday>=1 and sday<=30):
            valid_sday = 1
    elif(smonth==2):
        if(sday>=1 and sday<=29):
            valid_sday = 1
    if(emonth==1 or emonth==3 or emonth==5 or emonth==7 or emonth==8 or emonth==10 or emonth==12):
        if(eday>=1 and eday<=31):
            valid_eday = 1
    elif(emonth==4 or emonth==6 or emonth==9 or emonth==11):
        if(eday>=1 and eday<=30):
            valid_eday = 1
    elif(emonth==2):
        if(eday>=1 and eday<=29):
            valid_eday = 1
    if(valid_sday==1 and valid_eday==1):
        valid_day = 1
    
    if(valid_day==1):
        if(eyear==syear and emonth==smonth and eday<sday):
            valid_day = 0

        
    valid_hour = 0
    if(shour>=0 and shour<=23 and ehour>=0 and ehour<=23):
        valid_hour = 1
        
    valid_minute = 0
    if(sminute>=0 and sminute<=59 and eminute>=0 and eminute<=59):
        valid_minute = 1
        
    valid_dow = 0
    if(dow>=0 and dow<128):
        valid_dow = 1
    
    valid_uses_left = 0
    if(uses_left>=-1):
        valid_uses_left = 1
    
    
    if(valid_uses_left==1 and valid_dow==1 and valid_minute==1 and valid_hour==1 and valid_day==1 and valid_month==1 and valid_year==1):
        return 1
    else:
        return 0
            
def Check_Time_QR(cond):
    now = datetime.datetime.now()
    dow = now.strftime('%a')
    
    syear = smonth = sday = eyear = emonth = eday = ''
    shour = sminute = ehour = eminute = ''
    
    dash = 0
    for char in cond[0]:
        if(char == '-'):
            dash += 1
        else:
            if(dash == 0):
                syear += char
            if(dash == 1):
                smonth += char
            if(dash == 2):
                sday += char
    syear = int(syear)
    smonth = int(smonth)
    sday = int(sday)
    
        
    dash = 0
    for char in cond[1]:
        if(char == '-'):
            dash += 1
        else:
            if(dash == 0):
                eyear += char
            if(dash == 1):
                emonth += char
            if(dash == 2):
                eday += char
    eyear = int(eyear)
    emonth = int(emonth)
    eday = int(eday)
        
    dash = 0
    for char in cond[2]:
        if(char == '-'):
            dash += 1
        else:
            if(dash == 0):
                shour += char
            if(dash == 1):
                sminute += char
    shour = int(shour)
    sminute = int(sminute)
        
    dash = 0
    for char in cond[3]:
        if(char == '-'):
            dash += 1
        else:
            if(dash == 0):
                ehour += char
            if(dash == 1):
                eminute += char
    ehour = int(ehour)
    eminute = int(eminute)
    
    if(dow == 'Sun'):
        dow = 0
    if(dow == 'Mon'):
        dow = 1
    if(dow == 'Tue'):
        dow = 2
    if(dow == 'Wed'):
        dow = 3
    if(dow == 'Thu'):
        dow = 4
    if(dow == 'Fri'):
        dow = 5
    if(dow == 'Sat'):
        dow = 6

    date = 0
    if(syear<now.year and now.year<eyear):
        date = 1
    else:
        if(syear<now.year and now.year<eyear):
            if(smonth<now.month and now.month<emonth):
                date = 1
            elif(smonth==now.month and now.month==emonth):
                if(sday<=now.day and now.day<=eday):
                    date = 1
            elif(smonth==now.month and sday<=now.day):
                date = 1
            elif(now.month<emonth and now.day<=eday):
                date = 1
        elif(syear==now.year):
            if(smonth<now.month):
                date = 1
            elif(smonth==now.month and sday<=now.day):
                date = 1     
        elif(now.year==eyear):
            if(now.month<emonth):
                date = 1
            elif(now.month==emonth and now.day<=eday):
                date = 1
    
    time = 0
    if(shour<now.hour and now.hour<ehour):
        time = 1
    elif(now.hour == shour and now.hour == ehour):
        if(sminute<=now.minute and now.minute<=eminute):
            time = 1
    elif(now.hour == shour and sminute<=now.minute):
        time = 1
    elif(now.hour == ehour and now.minute<=eminute):
        time = 1

    check = int(cond[4],2) & (64 >> dow)
    day_of_week = 0
    if(check != 0):
        day_of_week = 1
    
#    print(cond)
#    print('Date:',date)
#    print('Time:',time)
#    print('DOW:',day_of_week)
    
    if(date==1 and time==1 and day_of_week==1):
        return 1
    else:
        return 0

#------------------------------------------------------    
      
def Verify_QR(qrid):
    Update_Dict()

    hash_exist = 0
    for i in QR:
        if(bcrypt.checkpw(qrid.encode('utf8'),i.encode('utf8')) == True):
            hash_exist = 1
            hqrid = i
    
    if(hash_exist == 1):
#        print("Hash Confirmed to Exist.")
        cond = QR[hqrid]
        check_time = Check_Time_QR(cond)
        if(check_time==1):
#            print("Time Within Bounds! With Uses Left!")
            if(int(cond[5])>1):
                QR[hqrid][5] = int(QR[hqrid][5])
                QR[hqrid][5] -= 1
                QR[hqrid][5] = str(QR[hqrid][5])
#                print('Uses Left:',QR[hqrid][5])
            elif(int(cond[5])==1):
#                print('Uses Left: 0')
                del QR[hqrid]
#            print('Infinite Uses Left')
            Update_File()
            return 1
        else:
            return 0
    else:
#        print("Hash Confirmed to NOT Exist")
        return 0
#------------------------------------------------------





