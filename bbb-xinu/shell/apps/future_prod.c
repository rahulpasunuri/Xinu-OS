#include<prodcons.h>
int n;
uint32 future_prod(future *fut) 
{
      int i, j;
      j = 0;
      for (i=0; i<100; i++) {
        j += i;
      }
      future_set(fut, &j);
      return OK;
}
