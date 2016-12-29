/* bootpack‚ÌƒƒCƒ“ */

#include "bootpack.h"
#include <stdio.h>

#define MEMMAN_FREES		4090	/* 大约32KB */
#define MEMMAN_ADDR			0x003c0000

struct FREEINFO {	/* 空闲块信息 */
	unsigned int addr, size;
};

struct MEMMAN {		/* 内存管理 */
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my, i;
	unsigned int memtotal;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

	init_gdtidt();
	init_pic();
	io_sti(); 
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	io_out8(PIC0_IMR, 0xf9); 
	io_out8(PIC1_IMR, 0xef); 

	init_keyboard();
	enable_mouse(&mdec);
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	init_palette();
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

	for (;;) {
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		} else {
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
			} else if (fifo8_status(&mousefifo) != 0) {
				i = fifo8_get(&mousefifo);
				io_sti();
				if (mouse_decode(&mdec, i) != 0) {
					/* データが3バイト揃ったので表示 */
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
					/* 鼠标指针的移动 */
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15); /* 隐藏鼠标 */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 16) {
						mx = binfo->scrnx - 16;
					}
					if (my > binfo->scrny - 16) {
						my = binfo->scrny - 16;
					}
					sprintf(s, "(%3d, %3d)", mx, my);
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15); /* 隐藏鼠标 */
					putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s); /* 显示坐标 */
					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); /* 描画鼠标 */
				}
			}
		}
	}
}

#define EFLAGS_AC_BIT     0x00040000
#define CR0_CACHE_DISABLE 0x60000000
unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;
	
	//确认cpu是 386+
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT;
	io_store_eflags(eflg);
	
	eflg = io_load_eflags();
	if((eflg & EFLAGS_AC_BIT)!=0)
	{
		flg486 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT;
	io_store_eflags(eflg);
	
	if(flg486 != 0)
	{
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE;
		store_cr0(cr0);	
	}
	
	i = memtest_sub(start, end);
	
	if(flg486 != 0)
	{
		cr0 = load_cr0();
		cr0&= ~CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}
	
	return i;
}

void memman_init(struct MEMMAN *man)
{
	man->frees = 0;     // 可用信息数目
	man->maxfrees = 0;  // 用于观察可用状况，frees的最大值
	man->lostsize = 0;  // 释放失败的内存大小总和
	man->losts = 0;     // 释放失败的次数
	return;
}

unsigned int memman_total(struct MEMMAN *man)
{
	unsigned int i , t = 0;
	for( i = 0; i < man->frees; i++)
	{
		t+= man->free[i].size;
	}
	return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
/* 申请内存 */
{
	unsigned int i, a;
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].size >= size) {
			/* 找到分配位置 */
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if (man->free[i].size == 0) {
				/* 正好分配完了 */
				man->frees--;
				for (; i < man->frees; i++) {
					man->free[i] = man->free[i + 1]; /* 構造体の代入 */
				}
			}
			return a;
		}
	}
	return 0; /*分配失败*/
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
/* 释放 */
{
	int i, j;
	/* 找到可以记录释放的位置 */
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].addr > addr) {
			break;
		}
	}
	/* free[i - 1].addr < addr < free[i].addr */
	if (i > 0) {
		/* 前边有数据 */
		if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
			/* 和前边的数据重合了 */
			man->free[i - 1].size += size;
			if (i < man->frees) {
				/*后边也有记录*/
				if (addr + size == man->free[i].addr) {
					/* 和后边的记录也重合了*/
					man->free[i - 1].size += man->free[i].size;
					/* 删除重合记录*/
					man->frees--;
					for (; i < man->frees; i++) {
						man->free[i] = man->free[i + 1]; /* 一次移动记录*/
					}
				}
			}
			return 0; /* 成功 */
		}
	}
	/* 前边无记录 */
	if (i < man->frees) {
		/* 后边有记录 */
		if (addr + size == man->free[i].addr) {
			/* 和后边重合了 */
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0; /* 成功 */
		}
	}
	/* 有足够空间保存记录 */
	if (man->frees < MEMMAN_FREES) {
		/* */
		for (j = man->frees; j > i; j--) {
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		if (man->maxfrees < man->frees) {
			man->maxfrees = man->frees; /* 更新最大值*/
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0; /* 成功 */
	}
	/* 记录失败 */
	man->losts++;
	man->lostsize += size;
	return -1; /* 失败 */
}
