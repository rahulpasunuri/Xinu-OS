#include<xinu.h>

struct QueueFuture
{
    int size; //Current size of the Queue.
    pid32 *p; //pointer to the pid queue..
    int capacity; //maximum size of the Queue..
    int left; //left index of the queue.
    int right; // right index of the queue..
};


int initializeQueue(struct QueueFuture* queue, int capacity);
void deallocateQueue(struct QueueFuture* queue);

//returns 1 on success, and -1 on failure..
int enqueueFuture(struct QueueFuture* queue, pid32 pid);
pid32 dequeueFuture(struct QueueFuture* queue);
