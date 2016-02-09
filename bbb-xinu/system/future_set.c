#include<future.h>

syscall future_set(future *f, int* value)
{
    if(f->flag == FUTURE_EXCLUSIVE)
    {
        f->value = *value;
        f->state = FUTURE_VALID;
        resume(f->pid);
    }
    else if (f->flag == FUTURE_SHARED)
    {
        if(f->state == FUTURE_VALID)
        {
            //future is already set..
            return SYSERR;
        }
        f->value = *value;
        f->state = FUTURE_VALID;
          
        //wake up all the process waiting for the future..
        int i;
        for(i= 0; i<f->get_queue.size; i++)
        {
            //resume(dequeueFuture(&f->get_queue));
            resume(f->get_queue.p[i]);
        }
    }
    else if(f->flag == FUTURE_QUEUE)
    {
        if(f->get_queue.size == 0)
        {
            //enqueue in the set_queue..
            enqueueFuture(&(f->set_queue), getpid());
            //suspend..
            suspend(getpid());
        }
        while(f->state == FUTURE_VALID)
        {
            resched();
        }
        f->value = *value;
        f->state = FUTURE_VALID;
        resume(dequeueFuture(&f->get_queue));
    }
    return OK;
}
