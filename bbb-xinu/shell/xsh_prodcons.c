#include <prodcons.h>
#include <stdlib.h>
//#include<future.h>
//#include "apps/produce.c"
//#include "apps/consume.c"
int n;                 //Definition for global variable 'n'
sid32 produced, consumed;
/*Now global variable n will be on Heap so it is accessible all the processes i.e. consume and produce*/

void printError()
{
    printf("Invalid usage of the command!!\n");
}

shellcmd xsh_prodcons(int nargs, char *args[])
{
    int i, count;
    //check for help..
    for( i=0; i<nargs; i++)
    {
        if(strncmp(args[i], "--help", 7) == 0)
        {
    		printf("Usage: %s\n\n", args[0]);
		    printf("Description:\n");
		    printf("\tA producer-consumer problem.\n");
		    printf("-f Uses futures.");
		    printf("Semaphores are used, if the -f option is not provided.");
		    printf("A valid natural number must be given as the last argument");
        }
    }
    
    int isFutureUsage = 0;
    for( i=0; i<nargs; i++)
    {
        if(strncmp(args[i], "-f", 3) == 0)
        {
            isFutureUsage = 1;
            break;
        }
    }



    if (!isFutureUsage)
    {

    //last argument will be the number..
    char* num = args[nargs-1];
    //check that the value entered is a number..
    i=0;
    if(num[0] == '\0')
    {
        printError();
        return 0;
    }
    i++;
    while(num[i] != '\0')
    {
        //printf ("%d \n ", num[i]);
        if(num[i] < 48 || num[i] > 57)
        {
              printError();
            return 0;
        }
        i++;
    } 
    //convert the char array to a number..
    n = atoi(num);

        //printf("Count value being used is: %d\n", count);
	    //check args[1] if present assign value to count
        consumed = semcreate(1);
        produced = semcreate(0);
	    //create the process producer and consumer and put them in ready queue.
	    //Look at the definations of function create and resume in exinu/system folder for reference.      
	    resume( create(producer, 1024, 20, "producer", 1, n) );
	    resume( create(consumer, 1024, 20, "consumer", 1, n) );
	}
	else
	{
        /*
        Part - 1 code.
        future *f1, *f2, *f3;

        f1 = future_alloc(FUTURE_EXCLUSIVE);
        f2 = future_alloc(FUTURE_EXCLUSIVE);
        f3 = future_alloc(FUTURE_EXCLUSIVE);

        resume( create(future_cons, 1024, 20, "fcons1", 1, f1) );
        resume( create(future_prod, 1024, 20, "fprod1", 1, f1) );
        ///*
        resume( create(future_cons, 1024, 20, "fcons2", 1, f2) );
        resume( create(future_prod, 1024, 20, "fprod2", 1, f2) );
        resume( create(future_cons, 1024, 20, "fcons3", 1, f3) );
        resume( create(future_prod, 1024, 20, "fprod3", 1, f3) );
        //*/
        
          future *f_exclusive, *f_shared, *f_queue;
 
          f_exclusive = future_alloc(FUTURE_EXCLUSIVE);
          f_shared = future_alloc(FUTURE_SHARED);
          f_queue = future_alloc(FUTURE_QUEUE);
         
        // Test FUTURE_EXCLUSIVE
          resume( create(future_cons, 1024, 20, "fcons1", 1, f_exclusive) );
          resume( create(future_prod, 1024, 20, "fprod1", 1, f_exclusive) );

        // Test FUTURE_SHARED
          resume( create(future_cons, 1024, 20, "fcons2", 1, f_shared) );
          resume( create(future_cons, 1024, 20, "fcons3", 1, f_shared) );
          resume( create(future_cons, 1024, 20, "fcons4", 1, f_shared) ); 
          resume( create(future_cons, 1024, 20, "fcons5", 1, f_shared) );
          resume( create(future_prod, 1024, 20, "fprod2", 1, f_shared) );

        // Test FUTURE_QUEUE
           ///*
          resume( create(future_cons, 1024, 20, "fcons6", 1, f_queue) );
          resume( create(future_cons, 1024, 20, "fcons7", 1, f_queue) );
          resume( create(future_cons, 1024, 20, "fcons7", 1, f_queue) );
          resume( create(future_cons, 1024, 20, "fcons7", 1, f_queue) );
          resume( create(future_prod, 1024, 20, "fprod3", 1, f_queue) );
          resume( create(future_prod, 1024, 20, "fprod4", 1, f_queue) );
          resume( create(future_prod, 1024, 20, "fprod5", 1, f_queue) );
          resume( create(future_prod, 1024, 20, "fprod6", 1, f_queue) );
          //*/
          
	}
	
	return 0;
}
