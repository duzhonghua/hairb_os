#include "bootpack.h"



#define EFLAGS_AC_BIT     0x00040000
#define CR0_CACHE_DISABLE 0x60000000
unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;
	
	//ȷ��cpu�� 386+
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
	man->frees = 0;     // ������Ϣ��Ŀ
	man->maxfrees = 0;  // ���ڹ۲����״����frees�����ֵ
	man->lostsize = 0;  // �ͷ�ʧ�ܵ��ڴ��С�ܺ�
	man->losts = 0;     // �ͷ�ʧ�ܵĴ���
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
/* �����ڴ� */
{
	unsigned int i, a;
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].size >= size) {
			/* �ҵ�����λ�� */
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if (man->free[i].size == 0) {
				/* ���÷������� */
				man->frees--;
				for (; i < man->frees; i++) {
					man->free[i] = man->free[i + 1]; /* ������δ��� */
				}
			}
			return a;
		}
	}
	return 0; /*����ʧ��*/
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
/* �ͷ� */
{
	int i, j;
	/* �ҵ����Լ�¼�ͷŵ�λ�� */
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].addr > addr) {
			break;
		}
	}
	/* free[i - 1].addr < addr < free[i].addr */
	if (i > 0) {
		/* ǰ�������� */
		if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
			/* ��ǰ�ߵ������غ��� */
			man->free[i - 1].size += size;
			if (i < man->frees) {
				/*���Ҳ�м�¼*/
				if (addr + size == man->free[i].addr) {
					/* �ͺ�ߵļ�¼Ҳ�غ���*/
					man->free[i - 1].size += man->free[i].size;
					/* ɾ���غϼ�¼*/
					man->frees--;
					for (; i < man->frees; i++) {
						man->free[i] = man->free[i + 1]; /* һ���ƶ���¼*/
					}
				}
			}
			return 0; /* �ɹ� */
		}
	}
	/* ǰ���޼�¼ */
	if (i < man->frees) {
		/* ����м�¼ */
		if (addr + size == man->free[i].addr) {
			/* �ͺ���غ��� */
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0; /* �ɹ� */
		}
	}
	/* ���㹻�ռ䱣���¼ */
	if (man->frees < MEMMAN_FREES) {
		/* */
		for (j = man->frees; j > i; j--) {
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		if (man->maxfrees < man->frees) {
			man->maxfrees = man->frees; /* �������ֵ*/
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0; /* �ɹ� */
	}
	/* ��¼ʧ�� */
	man->losts++;
	man->lostsize += size;
	return -1; /* ʧ�� */
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
