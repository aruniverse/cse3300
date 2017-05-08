#include <stdio.h>	/* for NULL */
#include <string.h>	/* for  strtok() */
#include <ctype.h>	/* for atoi() */
#include <errno.h>	/* for perror() */
#include <assert.h>	/* for assert() */

#include <netdb.h>	/* for gethostbyname() and struct hostent */
#include <sys/types.h>	/* for pid_t, socket.h */
#include <sys/param.h>	/* for MAXHOSTNAMELEN */
#include <sys/socket.h>	/* for AF_INET, etc. */
#include <netinet/in.h>	/* for struct sockaddr_in */

#include <time.h>
#include <sys/time.h>	/* for struct timeval */


/* this is in netinet/in.h; included here for reference only.
struct sockaddr_in {
	short	sin_family;
	u_short	sin_port;
	struct	in_addr sin_addr;
	char	sin_zero[8];
};
*/

/***************************************************************************/
static char *RCSId="$Id: c-client.c,v 1.1 1997/01/19 20:34:49 calvert Exp $";
/***************************************************************************/

#define NMSIZE		26
#define BUFSIZE		256

#define IDNAME	        "A.B.George"
#define USERNUM         5673

#define SVR_ADDR	"tao.ite.uconn.edu"	/* server name */
#define SVR_PORT	3300
/* Port # of lab 3 server */

/***************************************************************************/
/***************************************************************************/

void die(char *s)
{
	perror(s);
	exit(2);
}

char *getTime(void)
{
	struct timeval myTime;
	struct tm *myTimP;

	gettimeofday(&myTime,(struct timezone *)NULL);
	myTimP = localtime((time_t *)&myTime.tv_sec);
	return asctime(myTimP);
}

/*
 * translate strings in "dotted quad-port" notation ("W.X.Y.Z-P") to
 * struct sockaddr_in
 * NOTE: ranges not checked on integers.  Result is correct only
 * if the quad is actually valid, i.e. 0 <= W,X,Y,Z < 256
 */

int StringToSockaddr(char *name, struct sockaddr_in *address)
{
	int a,b,c,d,p;
	char string[BUFSIZE];
	register char *cp;

	assert(name!=NULL);
	assert(address!=NULL);
	
/* Copy the name string into a private buffer so we don't crash trying
 * to write into a constant string.
 */
	if (strlen(name) > BUFSIZE-1)
		return -1;
	else
		strcpy(string,name);

	cp = string;

	address->sin_family = AF_INET;

	/* throw away leading blanks, since they make gethostbyname() choke.  */
	while (cp[0]==' ' || cp[0]=='\t') cp++;

	/* is the first character a digit?
	 * If so, we assume "w.x.y.z-port"
	 * If not, we assume "hostname-port" */
	if (isdigit(cp[0])) {
		if (sscanf(cp,"%d.%d.%d.%d-%d",&a,&b,&c,&d,&p) != 5)
			return -2;

		address->sin_addr.s_addr = htonl(a<<24 | b<<16 | c<<8 | d);
		address->sin_port = htons(p);
	} else { 		/* we dont have a digit first */
		char *port;

		/* find the '-' in string: format must be hostname-port*/
		if ((port=strchr(cp,'-')) == NULL)
			return -3;

		/* split string in two... hostname\0port\0 and increment port past \0 */
		*port++ = '\0';

		/* look-up hostentry for the hostname */
		{
			struct hostent *destHostEntry;

			/* find the hostEntry for string */
			if ((destHostEntry=gethostbyname(cp))== NULL)
				return -4;

			/* copy the address from the hostEntry into our address */
			bcopy(destHostEntry->h_addr_list[0],
				&address->sin_addr.s_addr, destHostEntry->h_length);

		} /* look-up the hostentry for hostname */

		address->sin_port = htons(atoi(port));

	} /* else (we have hostname-port) */

	return 0;
}


/*
 * Convert a struct sockaddr_in into dotted.quad-port string notation.
 * String must point to a buffer of at least 22 characters.
 */
int SockaddrToString (char *string, struct sockaddr_in *ss)
{
	int ip = ss->sin_addr.s_addr;
	ip = ntohl(ip);
	if (string==0x0)
		return -1;
	sprintf(string ,"%d.%d.%d.%d-%d", (int)(ip>>24)&0xff,
		(int)(ip>>16)&0xff,
		(int)(ip>>8)&0xff,
		(int)ip&0xff, ntohs(ss->sin_port));
	return 1;
}

int main(int argc, char **argv)
{
        int mySocket, psock, psockPortNo, newSock;
	struct sockaddr_in destAddr, myAddr, addr2, newAddr;
	int lineSize, myPort;
	int sizeofmyAddr, sizeofdestAddr;
	int servnum, newservnum, lastservnum;
	char msgbuf[BUFSIZE], addrbuf[BUFSIZE], saddrbuf[BUFSIZE], rcvbuf1[BUFSIZE];
	char servNum[BUFSIZE], ackbuf[BUFSIZE], rcvbuf2[BUFSIZE];
	char addr2buf[BUFSIZE], saddrbuf2[BUFSIZE], msgbuf2[BUFSIZE], newservNum[BUFSIZE];
	char clientEndPoint[BUFSIZE], msgbuf3[BUFSIZE], rcvbuf3[BUFSIZE];
	
	char hostName[MAXHOSTNAMELEN+1];

	/***********************************************************
	 * Create a socket to be the endpoint of the connection
	 * to the server.  Set up the destination address information.
	 ************************************************************/

	if ((mySocket = socket(AF_INET,SOCK_STREAM,0)) < 0)
		die("couldn't allocate socket");
	
	/**************************************************************
	 * Make the connection
	 ****************************************************************/
	sprintf(addrbuf, "%s-%d", SVR_ADDR, SVR_PORT);
	StringToSockaddr (addrbuf, &destAddr);
	if (connect(mySocket,(struct sockaddr *) &destAddr,sizeof(destAddr)) < 0)
		die("failed to connect to server");

	printf("connected to server at %s\n",getTime());

	sizeofmyAddr = sizeof(myAddr);
	if (getsockname(mySocket, (struct sockaddr *) &myAddr,&sizeofmyAddr)<0) {
		printf("getsockname failed on mySocket!\n");
		addrbuf[0] = (char) 0;
	} else {
		/* set up addrbuf */
		SockaddrToString (addrbuf, &myAddr);
	}

	sizeofdestAddr = sizeof(destAddr);
	if (getpeername(mySocket,(struct sockaddr *) &destAddr,&sizeofdestAddr)<0) {
		printf("getpeername failed on mySocket!\n");
		saddrbuf[0] = (char) 0;
	} else {
		SockaddrToString (saddrbuf, &destAddr);
	}
	sprintf(msgbuf,"EX1 %s %s %d %s\n", saddrbuf, addrbuf, USERNUM, IDNAME);
	
	//a
	if ((psock = socket(AF_INET,SOCK_STREAM,0)) < 0)
	  die("couldn't allocate socket");
	//b
	addr2.sin_family = AF_INET;
	addr2.sin_addr.s_addr = htonl(INADDR_ANY);
	addr2.sin_port = htons(0);
	if (bind(psock,(struct sockaddr *)&addr2, sizeof(addr2)) < 0)
	  die("couldn't bind socket psock");
	//c
	int sizeofAddr2 = sizeof(addr2);
       	if (getsockname(psock, (struct sockaddr *) &addr2, &sizeofAddr2)<0) {
		printf("getsockname failed on psock!\n");
		addr2buf[0] = (char) 0;
	} else {
       		SockaddrToString (saddrbuf2, &addr2);
	}
       	printf("addr dotted.quad-port information: %s\n", addrbuf);
       	printf("psock dotted.quad-port information: %s\n", saddrbuf2);

	// get your ip address and psock portno then store it in a specific buffer
	int a, b, c;
	for(a=0; a<sizeof(addrbuf); a++)
	  {
	    if(addrbuf[a] == '-') //check if dash
	      {
		strncpy(clientEndPoint, addrbuf, ++a);  // get ip address
		for(b=sizeof(saddrbuf2); b>0; b--)      // get psock portno
		  {
		    if(saddrbuf2[b] == '-') //check if dash
		      {
			c = b+1;         // get beginIndex of portno
			for(c; c<sizeof(saddrbuf2); c++)
			  {
			    clientEndPoint[a] = saddrbuf2[c];
			    a++;
			  }
			break;
		      }
		  }
	       	break;
	      }
     	  }
	printf("Client Endpoint specifier: %s\n", clientEndPoint);
	
	//d
	listen(psock, 1); //psock will only be able to listen to 1 connection

	//e
       	sprintf(msgbuf2,"EX1 %s %s %d %s\n", saddrbuf, clientEndPoint, USERNUM, IDNAME);
	printf("%s\n", msgbuf2);
	if(send(mySocket, msgbuf2, sizeof(msgbuf2), 0) <0)
	  die("couldn't send properly");

	//g
	if(recv(mySocket, rcvbuf1, sizeof(rcvbuf1), 0) < 0)
	  die("couldn't receive");
	if(strstr(rcvbuf1, "OK") == NULL)
	  die("No OK found");
	else
	  printf("%s\n", rcvbuf1);
	
	// commented out my alg for parsing the server num, it works fine on mac but not on linux
	/*
        int i, j, k=0;
	for(i=sizeof(rcvbuf1)-1; i>0; i--)
	  {
	    if(rcvbuf1[i] == ' ') //check if last whitespace
	      {
		j = i+1;         // get beginIndex of servNum
		for(j; j<sizeof(rcvbuf1); j++)
		  {
		    servNum[k] = rcvbuf1[j];
		    k++;
		  }
		break;
	      }
     	  }
	printf("ServerNumber: %s\n", servnum);
	*/
	// better way to get the server number
	servnum = atoi(strtok(strrchr(rcvbuf1, ' ')+1, "\n"));
	printf("ServerNumber: %d\n", servnum);

	// h
	int sizeofnewAddr = sizeof(newAddr);
	if((newSock = accept(psock, (struct sockaddr*)&newAddr, &sizeofnewAddr)) < 0)
	  die("couldn't accept connections");

	// i
	if(recv(newSock, rcvbuf2, sizeof(rcvbuf2), 0) < 0)
	  die("couldn't receive message");
	printf("message received by newSock: %s\n", rcvbuf2);
	// commented out my alg for parsing the new server num, it works fine on mac but not on linux
	/*
        k=0;
	for(i=sizeof(rcvbuf2); i>0; i--)
	  {
	    if(rcvbuf2[i] == ' ') //check if last whitespace
	      {
		j = i+1;         // get beginIndex of newservNum
		for(j; j<sizeof(rcvbuf2); j++)
		  {
		    newservNum[k] = rcvbuf2[j];
		    k++;
		  }
		break;
	      }
     	  }
	//	printf("New ServerNumber: %s\n", newservNum);
	//	printf("CSE 3300 server sent %s\n", newservNum);
       	*/
	// better way to get the server number
	newservnum = atoi(strtok(strrchr(rcvbuf2, ' ')+1, "\n"));
	printf("CSE 3300 server sent %d\n", newservnum);
	
	// j
	//sprintf(msgbuf3,"%d %d\n", atoi(servNum)+1 , atoi(newservNum)+1);
	sprintf(msgbuf3,"%d %d\n", servnum+1 , newservnum+1);
	printf("Send String: %s\n", msgbuf3);
	if(send(newSock, msgbuf3, sizeof(msgbuf3), 0) < 0)
	  die("couldn't send");

	// k
	if(recv(mySocket, rcvbuf3, sizeof(rcvbuf3), 0) < 0)
	  die("couldnt receive");
	printf("message received by original connection: %s\n", rcvbuf3);
	
	lastservnum = atoi(strtok(strrchr(rcvbuf3, ' ')+1, "\n"));
	printf("servNum: %d \nnewservNum: %d \nlastservNum: %d \n", servnum,newservnum,lastservnum);
	
	close(mySocket);
	close(psock);
	
	return 0;
}
