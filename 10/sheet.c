#include "bootpack.h"

#define SHEET_USE		1

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
	struct SHTCTL *ctl;
	int i;
	ctl = (struct SHTCTL *)memman_alloc_4k(memman, sizeof(struct SHTCTL));
	if(ctl == 0)
	{
		goto err;
	}
	
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1; /* 设置成隐藏 */

	for(i = 0; i < MAX_SHEETS; i++)
	{
		ctl->sheets0[i].flags = 0;
	}
	
err:
	return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
	struct SHEET * sht;
	int i;
	for( i =0; i < MAX_SHEETS; i++)
	{
		if(ctl->sheets0[i].flags == 0)
		{
			sht = &ctl->sheets0[i];
			sht->flags = SHEET_USE; /* 使用 */
			sht->height = -1;       /* 隐藏 */
			return sht;
		}
	}
	return 0;// not found
}
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;
	return;
}

void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, c, *vram = ctl->vram;
	struct SHEET *sht;
	for (h = 0; h <= ctl->top; h++) {
		sht = ctl->sheets[h];
		buf = sht->buf;
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		
		if(bx0 < 0){bx0 = 0;}
		if(by0 < 0){by0 = 0;}
		
		if(bx1 > sht->bxsize){ bx1 = sht->bxsize;}
		if(by1 > sht->bysize){ by1 = sht->bysize;}
			
		if((sht->vy0 + by1) > ctl->ysize){ by1 = (ctl->ysize - sht->vy0);}
		if((sht->vx0 + bx1) > ctl->xsize){ bx1 = (ctl->xsize - sht->vx0);}
			
		for (by = by0; by < by1; by++) 
	  {	
			vy = sht->vy0 + by;
			for (bx = bx0; bx < bx1; bx++) 
			{
				vx = sht->vx0 + bx;		
				c = buf[by * sht->bxsize + bx];
				if (c != sht->col_inv) 
				{
					vram[vy * ctl->xsize + vx] = c;
				}
			}
		}
	}
	return;
}


void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height)
{
	int h, old = sht->height; /* 保存旧高度 */

	/* 改正不正确的高度设置 */
	if (height > ctl->top + 1) {
		height = ctl->top + 1;
	}
	if (height < -1) {
		height = -1;
	}
	sht->height = height; /* 设定高度 */

	/* 更换图层位置 */
	if (old > height) {	
	
	/* 比以前降低了 */
		if (height >= 0) {
			/* 新设定的是显示图层 */
			for (h = old; h > height; h--) {
				ctl->sheets[h] = ctl->sheets[h - 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		} else {/* 新设定的是隐藏图层 */
			if (ctl->top > old) {
				/* 之前的显示图层不是最低下一层，需要把下边的上移 */
				for (h = old; h < ctl->top; h++) {
					ctl->sheets[h] = ctl->sheets[h + 1];
					ctl->sheets[h]->height = h;
				}
			}
			ctl->top--; /* 显示图层数目减一 */
		}
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize); /* 更新显示 */
	} else if (old < height) {	/* 比以前高 */
		if (old >= 0) {
			/* 之前也是显示图层 */
			for (h = old; h < height; h++) {
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		} else {
		    /* 之前是隐藏图层 */
			/* 把新图层下边的下移 */
			for (h = ctl->top; h >= height; h--) {
				ctl->sheets[h + 1] = ctl->sheets[h];
				ctl->sheets[h + 1]->height = h + 1;
			}
			ctl->sheets[height] = sht;
			ctl->top++; /* 显示图层+1 */
		}
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize); /* 更新显示 */
	}
	return;
}

void sheet_refresh(struct SHTCTL *ctl, struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
	if (sht->height >= 0) { /* 如果正在显示，这按照新图层的信息刷新画面 */
		sheet_refreshsub(ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1);
	}
	return;
}

void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0)
{
	int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	
	if (sht->height >= 0) { /* 正在使用 */
		/* 更新画面旧画面 */
		sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0+sht->bxsize, old_vy0+sht->bysize); 
		/* 更新新画面 */
		sheet_refreshsub(ctl, vx0, vy0, vx0+sht->bxsize, vy0+sht->bysize); 
	}
	return;
}

void sheet_free(struct SHTCTL *ctl, struct SHEET *sht)
{
	if (sht->height >= 0) {
		sheet_updown(ctl, sht, -1); /* 隐藏图层 */
	}
	sht->flags = 0; /* 设置成未使用 */
	return;
}

