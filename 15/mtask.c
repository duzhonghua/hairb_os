#include "bootpack.h"
#include <stdio.h>
#ifdef MY_TASK
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
        taskman.tasks[r].runtime++;
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
        taskman.tasks[i].runtime = 0;
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
#else
struct TASKCTL *taskctl;
struct TIMER *task_timer;

struct TASK *task_init(struct MEMMAN *memman)
{
	int i;
	struct TASK *task;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	taskctl = (struct TASKCTL *) memman_alloc_4k(memman, sizeof (struct TASKCTL));
	for (i = 0; i < MAX_TASKS; i++) {
		taskctl->tasks0[i].flags = 0;
		taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks0[i].tss, AR_TSS32);
	}
	task = task_alloc();
	task->flags = 2; /* 動作中マーク */
	taskctl->running = 1;
	taskctl->now = 0;
	taskctl->tasks[0] = task;
	load_tr(task->sel);
	task_timer = timer_alloc();
	timer_settime(task_timer, 2);
	return task;
}

struct TASK *task_alloc(void)
{
	int i;
	struct TASK *task;
	for (i = 0; i < MAX_TASKS; i++) {
		if (taskctl->tasks0[i].flags == 0) {
			task = &taskctl->tasks0[i];
			task->flags = 1; /* 使用中マーク */
			task->tss.eflags = 0x00000202; /* IF = 1; */
			task->tss.eax = 0; /* とりあえず0にしておくことにする */
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.ldtr = 0;
			task->tss.iomap = 0x40000000;
			return task;
		}
	}
	return 0; /* もう全部使用中 */
}

void task_run(struct TASK *task)
{
	task->flags = 2; /* 動作中マーク */
	taskctl->tasks[taskctl->running] = task;
	taskctl->running++;
	return;
}

void task_switch(void)
{
	timer_settime(task_timer, 2);
	if (taskctl->running >= 2) {
		taskctl->now++;
		if (taskctl->now == taskctl->running) {
			taskctl->now = 0;
		}
		farjmp(0, taskctl->tasks[taskctl->now]->sel);
	}
	return;
}
#endif