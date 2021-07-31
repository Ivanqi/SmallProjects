%include "ldrasm.inc"
global _start
global realadr_call_entry
global IDT_PTR
global ldrkrl_entry
[section .text]
[bits 32]
_start:
_entry:
	cli
	lgdt [GDT_PTR]                  ; 加载GDT地址到GDTR寄存器
	lidt [IDT_PTR]                  ; 加载IDT地址到IDTR寄存器
	jmp dword 0x8 :_32bits_mode     ; 长跳转刷新CS影子寄存器