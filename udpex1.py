# Description: Working with UDP sockets part 2 Server Under Test
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

# step 1
print("creating database")
dataBase = {}                           # creating dictionary / hashtable of SSN, POBox
with open('db.txt', 'r') as dataFile :
    for line in dataFile :
        ssn, poBox = line.strip().split('\t')
        dataBase[int(ssn)] = poBox
#print(dataBase)    # used for testing, to make sure db is correct
# step 2 & 3
serverSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # create UDP socket
serverSock.bind(('', 2786))
print("server is running")
#step 4
cont = True
logFile = open('serverLogFile.txt', 'w')   #write to server log file
iterations = 0
while cont :
    servRcvMes, sendAddr = serverSock.recvfrom(1024)          # get incoming message and address of CSE3300 server
    iterations += 1
    print("iteration: ", iterations)
    sRM = "R: " + str(servRcvMes) + " " + str(len(servRcvMes)) + "\n"   # sRM = server recv message
    print(sRM)
    logFile.write(sRM)
    rcvCheckMes = struct.unpack(">HBBLLHh", servRcvMes)
    # 0 H = 0,0,3300
    # 1 B = 04
    # 2 B = 08
    # 3 L = client cookie
    # 4 L = request data SSN
    # 5 H = checksum
    # 6 h = TO + result
    rcvCheck = struct.pack(">HBBLLHh", rcvCheckMes[0], rcvCheckMes[1], rcvCheckMes[2], rcvCheckMes[3], rcvCheckMes[4], 0, rcvCheckMes[6])

    errors = [False, False, False, False]
    # errors = checksum error, syntax error, unknown ssn error, server error
    if 4 != rcvCheckMes[1] :                 #check if lab # is the same: 4,8 = 0000 0100 0000 1000 = 1032
        labNumVal = struct.pack(">B", rcvCheckMes[1])
        print("Incorrect Syntax Error, Lab Number is not the same!", 4, rcvCheckMes[1], labNumVal)
        errors[1] = True
    else :
        print("Correct lab num")

    if 8 != rcvCheckMes[2] :                 #check if lab # is the same: 4,8 = 0000 0100 0000 1000 = 1032
        verNumVal = struct.pack(">B", rcvCheckMes[2])
        print("Incorrect Syntax Error, Version Number is not the same!", 8, rcvCheckMes[2], verNumVal)
        errors[1] = True
    else :
        print("Correct version num")

    if checkSum(rcvCheck) == rcvCheckMes[5] :   # check if the checksums are the same
        print("Correct checksum")
    else :
        print("Incorrect Checksum Error")
        errors[0] = True
    ssn = rcvCheckMes[4]

    if ssn in dataBase :
        poBox = dataBase[ssn]
        print("SSN is in DB: ", ssn, poBox)
    else :
        print("Unknown SNN error ", ssn, " is not in DB!")
        errors[2] = True

    if True in errors :
        result = bytes([0x80, 0x00])
        if errors[0] :  # if error in checksum
            result = bytes([0x80, 0x01])
        if errors[2] :  # if unknown ssn error
            result = bytes([0x80, 0x03])
        if errors[1] :  # if error in syntax
            result = bytes([0x80, 0x02])
        print(result)
    else :
        result = struct.pack(">h", int(poBox))

    # work on sending response
    firstByte = bytes([0x4C, 0xE4]) # 0100 1100 1110 0100 = 0,1,3300
    labNum = struct.pack(">B", rcvCheckMes[1])
    version = struct.pack(">B", rcvCheckMes[2])
    cookie = struct.pack(">L", rcvCheckMes[3])
    reqData = struct.pack(">L", rcvCheckMes[4])
    checksum = struct.pack(">H", 0)
    sendMes = firstByte + labNum + version + cookie + reqData + checksum + result
    checksum = struct.pack(">H", int(checkSum(sendMes)))
    sendMes = firstByte + labNum + version + cookie + reqData + checksum + result
    serverSock.sendto(sendMes, sendAddr)
    sSM = "S: " + str(sendMes) + " " + str(len(sendMes)) + "\n"
    print(sSM) 
    logFile.write(sSM)

    # if iterations == 8 :
    #    cont = False
    #    break