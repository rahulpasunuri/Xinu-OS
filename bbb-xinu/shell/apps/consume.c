#include <prodcons.h>
#include <stdarg.h>
sid32 produced, consumed;
//int n;

void consumer(int count)
{
	//Code to consume values of global variable 'n' until the value
	//of n is less than or equal to count
	//print consumed value e.g. consumed : 8
	
    while(n<=count)
	{
	    wait(produced);
        kprintf("Consumed: %d\n", n);
        signal(consumed);
	}
    //kprintf("Consumed: %d\n", n);	
    semdelete(consumed);
    semdelete(produced);
}
