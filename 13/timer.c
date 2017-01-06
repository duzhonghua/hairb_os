
#include "bootpack.h"

#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040

struct TIMERCTL timerctl;

#define TIMER_FLAGS_ALLOC		1
#define TIMER_FLAGS_USING		2	
void init_pit(void)
{
	int i;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	timerctl.next = 0xffffffff; /*初始化*/
	timerctl.using = 0;
         timerctl.head = 0;
	for (i = 0; i < MAX_TIMER; i++) {
		timerctl.timers0[i].flags = 0; /* 未使用 */
	}
	return;
}

struct TIMER *timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timers0[i].flags == 0) {
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timers0[i];
		}
	}
	return 0; 
}

void timer_free(struct TIMER *timer)
{
	timer->flags = 0; 
	return;
}

void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data)
{
	timer->fifo = fifo;
	timer->data = data;
         timer->next = 0;
	return;
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	int e;
         struct TIMER *iter, *last= 0;
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	/* 找到插入的位置*/
        last = timerctl.head; 
        if(timerctl.head == 0){
            timerctl.head = timer;
        }
        else
        {   
	    for ( iter = timerctl.head; iter!=0;  ) {
                if (timer->timeout < iter->timeout) {
                    break;
                }
                last = iter;
                iter = iter->next;
	    }

             if(iter == timerctl.head){
                    timerctl.head = timer;
                    timer->next =  iter;
              }else{
                    last->next = timer;
                    timer->next = iter;
              }
             
       }
    
        if(timerctl.next > timer->timeout){
            timerctl.next  = timer->timeout;
        }    
        io_store_eflags(e);
        return;
}

void inthandler20(int *esp)
{
        struct TIMER *iter, *last= 0;
	io_out8(PIC0_OCW2, 0x60);	/* 通知PIC IRQ0处理完毕 */
	timerctl.count++;

        if (timerctl.next > timerctl.count) {
	 	/* 没有超时 */
		return;
	}
        
         last = timerctl.head; 
         
         for ( iter = timerctl.head; iter!=0; iter= iter->next ) {
            if ( timerctl.count < iter->timeout){
                   break;
            }
            iter->flags = TIMER_FLAGS_ALLOC;
            fifo32_put(iter->fifo,  iter->data);
         }

         timerctl.head = iter;

	if (timerctl.head > 0) {
		timerctl.next =  timerctl.head->timeout;
	} else {
		timerctl.next = 0xffffffff;
	}
	return;
}
