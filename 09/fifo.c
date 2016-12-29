#include "bootpack.h"
#define FLAGS_OVERRUN 0x0001



void fifo8_init(struct FIFO8* fifo, int size, unsigned char * buf){
#ifdef FIFO_OWN
	fifo->size = size;
	fifo->buf = buf;
	fifo->flags = 0;
	fifo->next_r = 0;
	fifo->next_w = 0;
#else	
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size;
	fifo->flags = 0;
	fifo->p = 0;
	fifo->q = 0;
	 return;	
#endif
}

int fifo8_put(struct FIFO8 *fifo, unsigned char data){
	
#ifdef FIFO_OWN
	fifo->buf[(fifo->next_w++)%(fifo->size)] = data;
	fifo->next_w%= fifo->size;
	
	if(fifo->next_w == fifo->next_r){
		fifo->next_r = (fifo->next_r+1)%fifo->size;
	}
#else
	
	if( fifo->free == 0){
		fifo->flags |= FLAGS_OVERRUN;
		return -1;	
	}
	
	fifo->buf[fifo->p] = data;
	fifo->p++;
	
	if(fifo->p == fifo->size){
		fifo->p = 0;	
	}
	fifo->free--;
	
	return 0;
#endif
}

int fifo8_get( struct FIFO8* fifo)
{
	 int data;
#ifdef FIFO_OWN
	if( fifo->next_r == fifo->next_w)
	{
		return -1;
	}
	else
	{
		data = fifo->buf[(fifo->next_r++)%fifo->size];
		fifo->next_r %= fifo->size;
	}
	
	return data;
#else

	if(fifo->free == fifo->size)
  {
  	return -1;
  }	
  	
  data = fifo->buf[fifo->q];
  fifo->q++;
  
  if(fifo->q == fifo->size)
 	{
 		fifo->q = 0;
 	}
 	fifo->free++;
 	return data;
#endif
}
int fifo8_status(struct FIFO8 *fifo)
{
	#ifdef FIFO_OWN
	if(fifo->next_w >= fifo->next_r)
	{
		return fifo->next_w - fifo->next_r;
	}
	else
	{
		return fifo->next_w + fifo->size - fifo->next_r;
	}
	
	#else
	return fifo->size - fifo->free;	
	#endif
}

void show_all_list(struct FIFO8 *fifo)
{
	
}