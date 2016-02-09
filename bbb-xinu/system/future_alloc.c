#include<future.h>

future* future_alloc(int future_flag)
{
    future* obj = (future* )getmem(sizeof(future));
    //check for syserror..
    ///*
    if( (char*)obj == (char *)SYSERR)
    {
        kprintf("Error in Allocating Memory for Future!!");
        return NULL;
    }
    //set the object flag..
    obj->flag = future_flag;
    
    //initialize both the set and get queues..
    int status1 = initializeQueue(&(obj->set_queue), 100); //setting the max size of 100 for now..
    int status2 = initializeQueue(&(obj->get_queue), 100);
    
    if (status1 == -1 || status2 == -1)
    {
        freemem((char*)obj, sizeof(obj)); // free the memory used for the future..
        kprintf("Error in Allocating Memory for the Queues!!");
        return NULL;        
    }
    
    //set the object state..
    obj->state = FUTURE_EMPTY;
    return obj;
}
