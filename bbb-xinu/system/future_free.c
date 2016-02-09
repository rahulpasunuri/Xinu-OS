#include<future.h>

syscall future_free(future* f)
{
    //free the set queue..
    deallocateQueue(&(f->set_queue));
    //free the get queue..
    deallocateQueue(&(f->get_queue));
    
    //free the future..
    return freemem((char*)f, sizeof(f));
}
