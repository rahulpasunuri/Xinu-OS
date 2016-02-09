#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>


#define MY_PORT 20000
#define BUFSIZE 2048
#define BUFLEN 2048

int senderTerminated = 0;
int receiverTerminated = 0;

char* remoteIp;
int remotePort;
int fd;

void printError()
{
    printf("Invalid Usage of the command");
    printHelp();
    exit(1);
}

void printHelp()
{
    printf("Use Remote IP address as the first parameter, and the remote port as the second parameter!!\n");
    printf("Use --help to get this message!\n");
}

void receive_udp()
{
	struct sockaddr_in myaddr;	/* our address */
	struct sockaddr_in remaddr;	/* remote address */
	socklen_t addrlen = sizeof(remaddr);		/* length of addresses */
	int recvlen;			/* # bytes received */
	int msgcnt = 0;			/* count # of messages we received */
	unsigned char buf[BUFSIZE];	/* receive buffer */

	/* create a UDP socket */
    /*
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return;
	}
    */
	/* bind the socket to any valid IP address and a specific port */

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(MY_PORT);

    /*
	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return;
	}
    */
    
	/* now loop, receiving data and printing what we received */
	while(senderTerminated == 0) {
		printf("waiting on port %d\n", MY_PORT);
		recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
		if (recvlen > 0) {
			buf[recvlen] = 0;
			//get the corresponding string from the message..
			char msg[1500];
			int pos = 0;
			msg[recvlen] = '\0';
			for(; pos<recvlen; pos++)
			{
			   msg[pos] = buf[pos]; 
			}
			printf("Received Message is: %s\n", msg);
            //send_message();
		}
        else
        {
            printf("Received Length of Bytes is < 0!!\n");
        }
	}
	receiverTerminated = 1;
}


void send_message()
{

	struct sockaddr_in myaddr, remaddr;
	int fd, i, slen=sizeof(remaddr);
	char buf[BUFLEN] = "DEADBEEF";	/* message buffer */
	int recvlen;		/* # bytes in acknowledgement message */
	char *server = remoteIp;	/* change this to use a different server */

	/* create a socket */

	if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
		printf("socket created\n");

	/* bind it to all local addresses and pick any port number */

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(0);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return;
	}       
 
	/* now define remaddr, the address to whom we want to send messages */
	/* For convenience, the host address is expressed as a numeric IP address */
	/* that we will convert to a binary format via inet_aton */

	memset((char *) &remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(remotePort);
	if (inet_aton(server, &remaddr.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
    
	if (sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&remaddr, slen)==-1) {
		perror("sendto");
	}
	printf("Packet with message %s sent!!\n", buf);

	close(fd);
	senderTerminated = 0;
}


void send_udp()
{
	struct sockaddr_in myaddr, remaddr;
	int i, slen=sizeof(remaddr);
	char buf[BUFLEN];	/* message buffer */
	int recvlen;		/* # bytes in acknowledgement message */
	char *server = remoteIp;	/* change this to use a different server */

	/* create a socket */
    /*
	if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
		printf("socket created\n");
    */
	/* bind it to all local addresses and pick any port number */

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(MY_PORT);


    /*
	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return;
	} 
	*/      

	/* now define remaddr, the address to whom we want to send messages */
	/* For convenience, the host address is expressed as a numeric IP address */
	/* that we will convert to a binary format via inet_aton */

	memset((char *) &remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(remotePort);
	if (inet_aton(server, &remaddr.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

	/* now let's send the messages */

	while(1) {
		scanf("%s", buf);
		if(strcmp(buf, "exit") == 0)
		{
		    break;
		}
		if (sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&remaddr, slen) < 0) {
			printf("THis line is printing\n");
			perror("sendto");
		}
		printf("Packet with message %s sent!!\n", buf);
	}
	close(fd);
	senderTerminated = 0;
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--help") == 0)
    {
        printHelp();
        return 0;
    }
    if(argc != 3)
    {
        printError();
    }
    
    //check the validity of the ip address..
	struct sockaddr_in testaddr;
	if (inet_aton(argv[1], &testaddr.sin_addr)==0) {
        printError();
	}
    //check the validity of the port number..
    char* port_str = argv[2];
    int pos = 0;
    if (strlen(port_str)==0)
    {
        printError();
    }

    for(; port_str[pos]!= '\0'; pos++)
    {
        int currDigit = (int) port_str[pos] -   48   ;
        if(currDigit < 0 || currDigit > 9)
        {
            printError();
        }
    }
    
    pthread_t sender_thread;
    pthread_t receiver_thread;

    remoteIp = argv[1];
    remotePort = atoi(argv[2]);
	
	struct sockaddr_in myaddr;
	char *server = remoteIp;	/* change this to use a different server */
    
	if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
		printf("socket created\n");

	/* bind it to all local addresses and pick any port number */

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(MY_PORT);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return;
	}    
    
    
    int ret1 = pthread_create(&sender_thread, NULL, send_udp, NULL);
    int ret2 = pthread_create(&receiver_thread, NULL, receive_udp, NULL);
    //int ret1 = 0;
    if(ret2 || ret1) {
        fprintf(stderr, "Error creating threads\n");
        return 1;
    }

    /* wait for the second thread to finish */
    ///*
    if(pthread_join(sender_thread, NULL)) {
        fprintf(stderr, "Error joining to sender thread\n");
    }
    //*/
    
    if(pthread_join(receiver_thread, NULL)) {
        fprintf(stderr, "Error joining to sender thread\n");
    }

    return 0;
}
