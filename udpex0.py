# Description: Working with UDP sockets part 1
# Author: Arun George
# Class: UConn, CSE 3300, Spring 2017
# Instructor: Song Han
# TA: Tao Gong

import socket, random, struct

def checkSum(message):
    checksum = 0
    for i in range(0, 16, 2):
        w = (message[i] << 8) + (message[i + 1])
        checksum += w
    checksum = (checksum >> 16) + (checksum & 0xFFFF)
    checksum = ~checksum&0xFFFF
    return checksum

mySocket =  socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # create UDP socket
destAddr = ('tao.ite.uconn.edu',3301)
#destAddr = ('137.99.132.10',2786) # was using to test out SUT initially

cont = True
while cont :
    reqType = input("Request type [0 or 1]: ")
    if int(reqType) == 1 :
        print("Request Type 1 aka get response")
        print("This part hasnt been impleneted yet, will be implented in udpex1")
        cont = False             # this part hasnt been impleneted yet
        break                   # will be implented in udpex1.py and udpex1client.py
    elif int(reqType) == 0:
        print("Request Type 0 aka send request")
        number = input("Enter SSN [987654321]: ")
        if int(number) == 0 :
            number = 987654321
        reqSSN = struct.pack(">i", int(number))
        result = struct.pack(">h", 0)
    else:
        cont = False
        break  

    topLine = bytes([0x0C, 0xE4, 0x04, 0x08]) # 0000 1100 1110 0100 0000 0100 0000 1000 = 0,0,3300,4,8
    myCookie1 = struct.pack(">h", random.randint(0,256)) # > = bigendian
    myCookie2 = struct.pack(">h", random.randint(0,256)) # > = bigendian
    myCookie = myCookie1 + myCookie2
    myCookieVal = struct.unpack(">L", myCookie)

    checksum = struct.pack(">H", 0)
    message = topLine + myCookie + reqSSN + checksum + result
    checksum = struct.pack(">H", int(checkSum(message)))
    message = topLine + myCookie + reqSSN + checksum + result

    mySocket.sendto(message, destAddr)
    print("S: ", message, len(message))
    mySocket.settimeout(5.0)
    rcvMes = mySocket.recv(1024)
    mySocket.settimeout(None)
    print("R: ", rcvMes, len(rcvMes))
    rcvCheckMes = struct.unpack(">HHLLHh", rcvMes)
    rcvCheck = struct.pack(">HHLLHh", rcvCheckMes[0], rcvCheckMes[1], rcvCheckMes[2], rcvCheckMes[3], 0, rcvCheckMes[5])

    if 1032 != rcvCheckMes[1] :         #check if lab version is the same: 4,8 = 0000 0100 0000 1000 = 1032
        print("Error, Lab Version is not the same!", 1032, rcvCheckMes[1])
        break
    if myCookieVal[0] != rcvCheckMes[2] : #check if cookies are the same
        print("Error, cookies are not the same!", myCookieVal[0], rcvCheckMes[2])
        break
    if checkSum(rcvCheck) == rcvCheckMes[4] :   # check if the checksums are the same
        print("PO Box: ", rcvCheckMes[5])
   #     print('done with ex0')