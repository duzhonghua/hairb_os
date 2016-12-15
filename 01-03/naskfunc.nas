; naskfunc
; TAB=4

[FORMAT "WCOFF"]   ; 制作目标文件的格式
[BITS 32]          ; 制作32为模式的机器语言

; 制作目标文件的信息
[FILE “nasfunc.nas"] ; 源文件名信息
    GLOBAL _io_hlt   ; 程序中包含的函数名
    
; 以下是实际的函数
[SECTION .text]             ; 目标文件中写了这些之后再写程序
_io_hlt:      ; void io_hlt(void);
    HLT
    RET