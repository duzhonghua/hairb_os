; haribote-os
; TAB=4
; BOOT_INFO
    CYLS  EQU 0x0ff0 ; 设定启动区
    LEDS  EQU 0x0ff1 ; 键盘灯状态
    VMODE EQU 0x0ff2 ; 关于颜色数目的信息，颜色的位数
    SCRNX EQU 0x0ff4 ; 分辨率X
    SCRNY EQU 0x0ff6 ; 分辨率Y
    VRAM  EQU 0x0ff8 ; 图像缓冲区开始地址
       
    ORG 0xc200      ; 指定程序内存加载位置
    
    MOV AL, 0x13    ; VGA显卡  320x200x8位彩色
    MOV AH, 0x00
    INT 0x10
    MOV BYTE [VMODE], 8
    MOV WORD [SCRNX], 320
    MOV WORD [SCRNY], 200
    MOV DWORD [VRAM], 0x000a0000
    
; 用BIOS取得键盘上各种LED指示灯的状态
    MOV AH, 0x02
    INT 0x16
    MOV [LEDS], AL 

fin:
  HLT
  JMP fin