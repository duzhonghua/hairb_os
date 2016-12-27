#include "bootpack.h"
#include <stdio.h>

void init_pic(void)
{
	io_out8( PIC0_IMR, 0xff); // �ر������ж�
	io_out8( PIC1_IMR, 0xff); // �ر������ж� 
	
	io_out8( PIC0_ICW1, 0x11); // ���س���ģʽ
	io_out8( PIC0_ICW2, 0x20); // IRQ0-7��INT20-27����
	io_out8( PIC0_ICW3, 1 <<2); // PIC1��IRQ2����
	io_out8( PIC0_ICW4, 0x01); // �޻�����ģʽ
	
	io_out8( PIC1_ICW1, 0x11); // ���ش���ģʽ
	io_out8( PIC1_ICW2, 0x28); // IRQ8-15��INT28-2f����
	io_out8( PIC1_ICW3, 2); // PIC1��IRQ2����
	io_out8( PIC1_ICW4, 0x01); // �޻�����ģʽ
	
	io_out8( PIC0_IMR, 0xfb); // 11111011 PIC1����ȫ����ֹ
	io_out8( PIC1_IMR, 0xff); // 11111111 ��ֹ�����ж�
	
	return;
}

#define PORT_KEYDAT		0x0060

struct FIFO8 keyfifo;
void inthandler21(int *esp)
{
	io_out8(PIC0_OCW2, 0x61);
	
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61);	
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&keyfifo, data);
	
	return;
}
struct FIFO8 mousefifo;
void inthandler2c(int *esp)
{
	io_out8(PIC1_OCW2, 0x64);
	io_out8(PIC0_OCW2, 0x62);
	fifo8_put(&mousefifo, io_in8(PORT_KEYDAT));
	return;
}

void inthandler27(int *esp)
{
	io_out8(PIC0_OCW2, 0x67); 
	return;
}