//#include <stddef.h>
//#include <stdio.h>   
#include<future.h>
/*Global variable for producer consumer*/
extern int n; /*this is just declaration*/
extern sid32 produced, consumed;
/*function Prototype*/
void consumer(int count);
void producer(int count);

uint32 future_prod(future *fut);
uint32 future_cons(future *fut);
