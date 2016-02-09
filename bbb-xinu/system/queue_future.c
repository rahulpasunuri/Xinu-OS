#include<queue_future.h>

int initializeQueue(struct QueueFuture* queue, int capacity)
{
    queue->p = (pid32 * ) getmem(capacity * sizeof(pid32));
    if( (char*)queue->p == (char *)SYSERR)
    {
        //printf("Error in Initializing a Queue!!");
        return -1;
    }
    queue->capacity = capacity;
    queue->size = 0;
    queue->left = 0;
    queue->right = 0;
    return 1;
}

void deallocateQueue(struct QueueFuture* queue)
{
    queue->capacity = 0;
    queue->left = 0;
    queue->right = 0;
    queue->size = 0;
    freemem((char*)queue->p, sizeof(queue->size * sizeof(pid32)));    
}

int enqueueFuture(struct QueueFuture* queue, pid32 pid)
{
    if(queue->size == queue->capacity - 1)
    {
        kprintf("Queue is already full!!");
        return -1;
    }
    else
    {
        queue->p[queue->right] = pid;
        queue->right = ((queue->right) + 1)%(queue->capacity);
        (queue->size)++;
    }
    return 1;
}

pid32 dequeueFuture(struct QueueFuture* queue)
{
    //kprintf("Queue size %d\t%d\t%d", queue->size,queue->left, queue->right);
    if(queue->size == 0)
    {
        return -1;
    }
    pid32 val = queue->p[queue->left];
    queue->left = (queue->left + 1) % (queue->capacity);
    (queue->size)--;
    return val;
}
