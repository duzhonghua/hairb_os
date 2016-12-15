; hello-os
; TAB=4

    ORG 0x7c00 ; 指明程序装载地址
; 一下技术用于标准的FAT12格式的软盘

    JMP entry
    DB 0x90
    DB "HARIBOTE" ; 启动区的名称可以是任意8字节字符串
    DW 512        ; 每个扇区的大小，必须为512字节
    DB 1          ; 簇的大小必须为1个扇区
    DW 1          ; FAT的起始位置，一般从第一个扇区开始
    DB 2          ; FAT的个数，必须为2
    Dw 224        ; 个目录的大小，一般为224项
    DW 2880       ; 该磁盘的大小，必须为2880扇区
    DB 0xf0       ; 磁盘的种类，必须为0xf0
    DW 9          ; FAT的长度，必须是9扇区
    DW 18         ; 一个磁道有几个扇区，必须是18
    DW 2          ; 磁头数，必须是2
    DD 0          ; 不使用分区，必须是0
    DD 2880       ; 重写一次磁盘大小
    DB 0,0,0x29   ; 意义不明，固定
    DD 0xffffffff ; 卷标号码
    DB "HELLO-OS   " ; 磁盘的名称
    DB "FAT12   " ; 磁盘格式说明  
    RESB 18       ; 先空出18字节
 
; 程序主体

entry:
    MOV AX, 0         ;初始化寄存器
    MOV SS, AX
    MOV SP, 0X7c00
    MOV DS, AX
    MOV ES, AX
    
    MOV SI,msg
    
putloop:
    MOV AL, [SI]
    ADD SI, 1    ; 给SI加1
    CMP AL, 0
    
    JE fin
    MOV AH, 0x0E ; 显示一个字符
    MOV BX, 15   ; 指定字体颜色
    INT 0x10     ; 调用先卡BIOS
    JMP putloop

fin:
    HLT          ; 让CPU停止，等待指令
    JMP  fin     ; 无限循环
    
    
; 信息显示部分
msg:
    DB 0x0a, 0x0a ; 换行两次
    DB “hello, world”
    DB 0x0a
    DB 0

    RESB 0x7dfe-$
    
    DB 0x55, 0xaa