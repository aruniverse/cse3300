# Description: Working with UDP sockets part 2 Client
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
mySocket.connect(destAddr)          # not sure if this should be done cause its a UDP socket
#myAddr = socket.gethostbyname(socket.gethostname()) 
myAddr, myPort = mySocket.getsockname()
myPort = 2786
print("SUT address - port: ", myAddr, myPort)

cont = True
while cont :
    reqType = input("Request type [0 or 1]: ")
    if int(reqType) == 1 :
        reqData = socket.inet_aton(myAddr)
        udpPort= struct.pack(">h", myPort)
    elif int(reqType) == 0:
        print("Request Type 0 aka send request")
        print("This part hasnt been impleneted yet, will be implented in udpex0.py")
        cont = False            # this part hasnt been impleneted yet
        break                   # will be implented in udpex0.py
    else:
        cont = False
        break  

    topLine = bytes([0x8C, 0xE4, 0x04, 0x08]) # 1000 1100 1110 0100 0000 0100 0000 1000 = 1,0,3300,4,8
    myCookie1 = struct.pack(">h", random.randint(0,256)) # > = bigendian
    myCookie2 = struct.pack(">h", random.randint(0,256)) # > = bigendian
    myCookie = myCookie1 + myCookie2
    myCookieVal = struct.unpack(">L", myCookie)
    checksum = struct.pack(">H", 0)
    message = topLine + myCookie + reqData + checksum + udpPort
    checksum = struct.pack(">H", int(checkSum(message)))
    message = topLine + myCookie1 + myCookie2 + reqData+ checksum + udpPort

    mySocket.sendto(message, destAddr)
    print("S: ", message, len(message))
    #mySocket.settimeout(5.0)
    rcvMes = mySocket.recv(1024)
    #mySocket.settimeout(None)
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
        resultVal = struct.pack(">h", rcvCheckMes[5])
        print("PO Box: ", resultVal, rcvCheckMes[5])