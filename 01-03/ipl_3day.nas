; hello-os
; TAB=4
    CYLS EQU 10
    ORG 0x7c00 ; 指明程序装载地址
; 以下技术用于标准的FAT12格式的软盘

    JMP entry
    DB 0x90
    DB "HELLOIPL" ; 启动区的名称可以是任意8字节字符串
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
    DB "HARIBOTEOS " ; 磁盘的名称
    DB "FAT12   " ; 磁盘格式说明  
    RESB 18       ; 先空出18字节
 
; 程序主体

    ;DB 0xb8, 0x00, 0x00, 0x8e, 0xd0, 0xbc, 0x00, 0x7c
    ;DB 0x8e, 0xd8, 0x8e, 0xc0, 0xbe, 0x74, 0x7c, 0x8a
    ;DB 0x04, 0x83, 0xc6, 0x01, 0x3c, 0x00, 0x74, 0x09
    ;DB 0xb4, 0x0e, 0xbb, 0x0f, 0x00, 0xcd, 0x10, 0xeb
    ;DB 0xee, 0xf4, 0xeb, 0xfd
entry:
    MOV AX, 0         ; 初始化寄存器
    MOV SS, AX
    MOV SP, 0x7c00
    MOV DS, AX        ; DS是默认段寄存器，要将其清零，使用ES作为段寄存器
       
    MOV AX, 0x820
    MOV ES, AX
    MOV CH, 0   ; 柱面0
    MOV DH, 0   ; 磁头0
    MOV CL, 2   ; 扇区2
    
readloop:
    MOV SI, 0

retry:
    MOV AH, 0x02 ; 读盘
    MOV AL, 1    ; 一个扇区
    MOV BX, 0
    MOV DL, 0x00 ; A驱动器
    INT 0x13     ; 调用磁盘BIOS
    JNC next
    ADD SI, 1
    CMP SI, 5    ; 循环5次
    JAE error
    MOV AH, 0x00 ; 为了重置
    MOV DL, 0x00
    INT 0x13     ; 重置驱动器
    JMP retry
               
next:
   MOV AX, ES
   ADD AX, 0x0020  
   MOV ES, AX      ; 内存地址增加0x200, 寻址方式[ES*16]+BX
   ADD CL, 1       ; 增加扇区
   CMP CL, 18
   JBE readloop    ; 循环读入18个扇区
   MOV CL, 1
   ADD DH, 1       ; 更换磁头
   CMP DH, 2
   JB readloop     ; 循环读入2个磁头数据
   MOV DH, 0
   ADD CH, 1       ; 增加柱面
   CMP CH, CYLS    
   JB readloop     ; 循环读入10个柱面

   JMP		0xc200   ; 进入操作系统   

fin:
    HLT          ; 让CPU停止，等待指令
    JMP  fin     ; 无限循环

error:
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


    
    
; 信息显示部分
msg:
    DB 0x0a, 0x0a ; 换行两次
		DB		"load error"
    DB 0x0a
    DB 0

    RESB 0x7dfe-$
    
    DB 0x55, 0xaa