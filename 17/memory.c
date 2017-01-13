#include "bootpack.h"



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

unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
	unsigned int a;
	size = (size + 0xfff) & 0xfffff000;
	a = memman_alloc(man, size);
	return a;
}

int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memman_free(man, addr, size);
	return i;
}
