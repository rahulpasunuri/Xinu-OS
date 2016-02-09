/* xsh_udpecho.c - xsh_udpecho */

#include <xinu.h>
#include <stdio.h>
#include <string.h>
//#include<future.h>


int senderTerminated = 0;
int receiverTerminated = 0;

const uint16	echoport= 20000;		/* port number for UDP echo	*/
const uint16	locport	= 52743;	/* local port to use		*/
const int32	retries	= 3;		/* number of retries		*/
const int32	delay_msg	= 2000;		/* reception delay in ms	*/
int32	slot;			/* UDP slot to use		*/
uint32	remoteip;		/* remote IP address to use	*/

void sender()
{
    printf("Sender Init\n");
    
    while(1)
    {
        char msg[1500] = "XINU_SEND";    
    	int32	msglen;			/* length of outgoing message	*/
        int i =0;
        sleep(5);
        //scanf("%s", msg);
        //printf("asdsadfakdjflkdsfugklksender1\n");
        /*
        if(strncmp(msg, "--exit", 7) == 0)
        {
            senderTerminated = 1;
            //exit when a exit message is sent..
            break;
        }
        */
	    msglen = strnlen(msg, 1200);
	    
	    int retval = udp_send(slot, msg, msglen);
	    if (retval == SYSERR) 
	    {
		    fprintf(stderr, "error sending UDP \n");
		    break;
	    }
	    printf("Message sent!!\n");
    }
    senderTerminated = 1;
}


void receiver()
{
    printf("Receiver Init!!\n");
	char	inbuf[1500];		/* buffer for incoming reply	*/
    while(!senderTerminated)
    {    
	    int retval = udp_recv(slot, inbuf, sizeof(inbuf), delay_msg);
	    if (retval == TIMEOUT) {
		    //fprintf(stderr, "timeout...\n");
		    continue;
	    } else if (retval == SYSERR) {
		    fprintf(stderr, "error from udp_recv \n");
		    continue;
	    }
	    //udp_recvaddr(&)
		kprintf("Message Received: %s\n", inbuf);
    }
    receiverTerminated = 1;
}

shellcmd xsh_network(int nargs, char *args[])
{
    
	/* For argument '--help', emit help about the 'udpecho' command	*/
    
	if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
		printf("Use: %s  REMOTEIP\n\n", args[0]);
		printf("Description:\n");
		printf("Options:\n");
		printf("\tREMOTEIP:\tIP address in dotted decimal\n");
		printf("\t--help\t display this help and exit\n");
		return 0;
	}

	/* Check for valid IP address argument */

	if (nargs != 2) {
		fprintf(stderr, "%s: invalid argument(s)\n", args[0]);
		fprintf(stderr, "Try '%s --help' for more information\n",
				args[0]);
		return 1;
	}
   
	if (dot2ip(args[1], &remoteip) == SYSERR) {
		fprintf(stderr, "%s: invalid IP address argument\r\n",
			args[0]);
		return 1;
	}
    
	/* register local UDP port */

	slot = udp_register(remoteip, echoport, locport);
	if (slot == SYSERR) {
		fprintf(stderr, "%s: could not reserve UDP port %d\n", args[0], locport);
		return 1;
	}

    //launch the threads here..
    resume( create(sender, 1024, 20, "sender", 1) );
    resume( create(receiver, 1024, 20, "receiver", 1) );
    //sender();
    //receiver();
    while(receiverTerminated == 0 || senderTerminated == 0){};
    //wait for the sender and receiver threads to end..
    udp_release(slot);
	return 0;
}
