#include "bootpack.h"
#include <stdio.h>

struct TASKMAN taskman;
struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
#define GDT_BASE 3

void switch_task(int index){
    farjmp(0, index*8);
    return;
}

void task_switch()
{
   int r;

   r = taskman.running +1;
   if( r >= taskman.count ){
        r = 0;
   }

    if(r != taskman.running){
        
        if(taskman.tasks[taskman.running].status == TASK_RUNNING ){
            taskman.tasks[taskman.running].status = TASK_STOPPED;
        }
        
        taskman.running = r;
        taskman.tasks[r].status = TASK_RUNNING;
        switch_task(taskman.tasks[r].clt);
    }
    return;
}
struct TSS32* alloc_tss( int entry, int i){
   struct TSS32* tss_b = memman_alloc(memman, sizeof(struct TSS32));
   if(tss_b == 0){return 0;}
   
    tss_b->ldtr = 0;
    tss_b->iomap = 0x40000000;
    tss_b->eip = entry;
   // if(i !=0)
    {
    tss_b->eflags = 0x00000202; /* IF = 1; */
    tss_b->eax = 0;
    tss_b->ecx = 0;
    tss_b->edx = 0;
    tss_b->ebx = 0;
    tss_b->esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024;
    tss_b->ebp = 0;
    tss_b->esi = 0;
    tss_b->edi = 0;
    tss_b->es = 1*8;
    tss_b->cs = ( 2)*8;
    tss_b->ss = 1*8;
    tss_b->ds =1*8;
    tss_b->fs = 1*8;
    tss_b->gs = 1*8;}
    return tss_b;
}
void task_init()
{
    int i;
    for(i =0; i < MAX_TASK; i++){
        taskman.tasks[i].tss = 0;
        taskman.tasks[i].status = TASK_UNALLOCK;
    }
    taskman.running = 0;
    taskman.count = 0;
}

int task_regsister( int entry, int runing)
{
    int i;

    for(i =0; i < MAX_TASK; i++){
        if(taskman.tasks[i].status == TASK_UNALLOCK){
            taskman.tasks[i].status = (runing==1)?TASK_RUNNING:TASK_STOPPED;
            taskman.tasks[i].tss = alloc_tss(entry, i);
            taskman.tasks[i].clt = i+GDT_BASE;
            set_segmdesc(gdt + GDT_BASE + i , 103, taskman.tasks[i].tss, AR_TSS32);
            if(runing ==1){  
                taskman.running = i;
                load_tr( taskman.tasks[i].clt*8);
            }
            
             taskman.count++;
            return 0;
        }
    }

    return -1;
}