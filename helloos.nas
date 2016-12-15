; hello-os
; TAB=4

    ORG 0x7c00 ; ָ������װ�ص�ַ
; һ�¼������ڱ�׼��FAT12��ʽ������

    JMP entry
    DB 0x90
    DB "HELLOIPL" ; �����������ƿ���������8�ֽ��ַ���
    DW 512        ; ÿ�������Ĵ�С������Ϊ512�ֽ�
    DB 1          ; �صĴ�С����Ϊ1������
    DW 1          ; FAT����ʼλ�ã�һ��ӵ�һ��������ʼ
    DB 2          ; FAT�ĸ���������Ϊ2
    Dw 224        ; ��Ŀ¼�Ĵ�С��һ��Ϊ224��
    DW 2880       ; �ô��̵Ĵ�С������Ϊ2880����
    DB 0xf0       ; ���̵����࣬����Ϊ0xf0
    DW 9          ; FAT�ĳ��ȣ�������9����
    DW 18         ; һ���ŵ��м���������������18
    DW 2          ; ��ͷ����������2
    DD 0          ; ��ʹ�÷�����������0
    DD 2880       ; ��дһ�δ��̴�С
    DB 0,0,0x29   ; ���岻�����̶�
    DD 0xffffffff ; �������
    DB "HELLO-OS   " ; ���̵�����
    DB "FAT12   " ; ���̸�ʽ˵��  
    RESB 18       ; �ȿճ�18�ֽ�
 
; ��������

    ;DB 0xb8, 0x00, 0x00, 0x8e, 0xd0, 0xbc, 0x00, 0x7c
    ;DB 0x8e, 0xd8, 0x8e, 0xc0, 0xbe, 0x74, 0x7c, 0x8a
    ;DB 0x04, 0x83, 0xc6, 0x01, 0x3c, 0x00, 0x74, 0x09
    ;DB 0xb4, 0x0e, 0xbb, 0x0f, 0x00, 0xcd, 0x10, 0xeb
    ;DB 0xee, 0xf4, 0xeb, 0xfd
entry:
    MOV AX, 0         ;��ʼ���Ĵ���
    MOV SS, AX
    MOV SP, 0X7c00
    MOV DS, AX
    MOV ES, AX
    
    MOV SI,msg
    
putloop:
    MOV AL, [SI]
    ADD SI, 1    ; ��SI��1
    CMP AL, 0
    
    JE fin
    MOV AH, 0x0E ; ��ʾһ���ַ�
    MOV BX, 15   ; ָ��������ɫ
    INT 0x10     ; �����ȿ�BIOS
    JMP putloop

fin:
    HLT          ; ��CPUֹͣ���ȴ�ָ��
    JMP  fin     ; ����ѭ��
    
    
; ��Ϣ��ʾ����
msg:
    DB 0x0a, 0x0a ; ��������
    DB ��hello, world��
    DB 0x0a
    DB 0

    RESB 0x7dfe-$
    
    DB 0x55, 0xaa