/* xsh_udpecho.c - xsh_udpecho */

//#include <xinu.h>
//#include <stdio.h>
#include<future.h>
#include <string.h>



int senderTerminated_future = 0;
int receiverTerminated_future = 0;

const uint16	echoport_future= 20000;		/* port number for UDP echo	*/
const uint16	locport_future	= 52743;	/* local port to use		*/
const int32	delay_msg_future	= 2000;		/* reception delay in ms	*/
int32	slot;			/* UDP slot to use		*/
uint32	remoteip;		/* remote IP address to use	*/
future *fut1=NULL;

void sendMessage(char* msg)
{
    //return;
    //f = future_alloc(FUTURE_EXCLUSIVE);
	int32	msglen;			/* length of outgoing message	*/
    msglen = strnlen(msg, 1200);
    int retval = udp_send(slot, msg, msglen);
    if (retval == SYSERR) 
    {
	    fprintf(stderr, "error sending UDP \n");
    }
    kprintf("Message sent!!\n");
}

void waitOnFuture(future* fut)
{
    int i, status;

    status = future_get(fut, &i);
    if (status < 1) {
        printf("future_get failed\n");
        return;
    }

    kprintf("Future Value is: %d\n", i);
}

void sender_future()
{
    fut1= future_alloc(FUTURE_EXCLUSIVE);
    //fut2= future_alloc(FUTURE_EXCLUSIVE);
    //fut3= future_alloc(FUTURE_EXCLUSIVE);
    kprintf("Sender Init\n");

    char msg[1500] = "Message1";
    sendMessage(msg);
    sleep(5);
    
    kprintf("Waiting for the Future value to be set!!\n");
    //waiting on futures to be set!!!
    waitOnFuture(fut1);
    
    senderTerminated_future = 1;
    //kprintf("sender exit\n");
}


void receiver_future()
{   
    //return;
    kprintf("Receiver Init!!\n");
	char	inbuf[1500];		/* buffer for incoming reply	*/
	int messageCount = 0;
    while(!senderTerminated_future)
    {    
	    int retval = udp_recv(slot, inbuf, sizeof(inbuf), delay_msg_future);
	    if (retval == TIMEOUT) {
		    //fprintf(stderr, "timeout...\n");
		    continue;
	    } else if (retval == SYSERR) {
		    fprintf(stderr, "error from udp_recv \n");
		    continue;
	    }
	    messageCount++;
	    int msgLen = strnlen(inbuf, 1500);
	   // if(messageCount == 1)
	    //{
	        future_set(fut1, &msgLen);
	        break;
	    //}
	    /*else if(messageCount == 2)
	    {
	        future_set(fut2, &msgLen);
	        break; //break after the 3 messages are received!!
	    }
	    else if(messageCount == 3)
	    {
	        future_set(fut3, &msgLen);
	        break; //break after the 3 messages are received!!
	    }*/
		kprintf("Message Received: %s\n", inbuf);
    }
    receiverTerminated_future = 1;
    //kprintf("Receiver exit\n");
}

shellcmd xsh_udpfuture(int nargs, char *args[])
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

	slot = udp_register(remoteip, echoport_future, locport_future);
	if (slot == SYSERR) {
		fprintf(stderr, "%s: could not reserve UDP port %d\n", args[0], locport_future);
		return 1;
	}

    //launch the threads here..
    resume( create(sender_future, 1024, 20, "sender", 1) );
    resume( create(receiver_future, 1024, 20, "receiver", 1) );
    //sender();
    //receiver();
    while( senderTerminated_future == 0){};
    //wait for the sender and receiver threads to end..
    udp_release(slot);
	//kprintf("Exiting the Program\n");
	return 0;
}
