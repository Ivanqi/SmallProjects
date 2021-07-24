/******************************************************************
*		ld自动化编译配置文件 Makefile			                     *
******************************************************************/

#include "config.h"

#ifdef CFG_X86_PLATFORM
#include "buildfile.h"

/* 物理地址 */
LINEAR_ADDRESS = 0x2000000;
/* 虚拟地址 */
VIRTUAL_ADDRESS = 0xffff800000000000; /*+0x1000000x400000;*/

/* 指定输出可执行文件的平台为 i386:x86-64 */
OUTPUT_ARCH(i386:x86-64)
OUTPUT_FORMAT(elf64-x86-64)

/*
	ENTRY伪指令用于指定汇编程序的入口点。将_start符号的值设置成入口地址
	_start 汇编程序的缺省入口，但是可以更改，想要更改其他标志，到相应的链接脚本中去用ENTRY指明其他入口标志
 */
ENTRY(_start)

/*
	.是定位器符号，可以对定位器符号赋值指定接下来内容的存储位置，如“ .= 0x10000”
	ALIGN(4): 返回一个基于location counter(.)的4字节对齐的地址值
 */
SECTIONS
{
	. =LINEAR_ADDRESS;
	__start_kernel = . + VIRTUAL_ADDRESS;

	/* 清除 .start.text 段 */
    __begin_start_text = .;
        .start.text : ALIGN(4) { *(.start.text) }
	__end_start_text = .;
	
	__begin_start_data = .;
        .start.data : ALIGN(0x4) { *(.start.data) }
	__end_start_data = .;
	
	__begin_start_data_pml4 = .;
        .start.data.pml4 : ALIGN(0x1000) { *(.start.data.pml4) }
	__end_start_data_pml4 = .;

	. +=VIRTUAL_ADDRESS;
	__begin_entry64_text = .;
        .entry64.text ALIGN(4) : AT(ADDR(.entry64.text)-VIRTUAL_ADDRESS){ *(.entry64.text) }
	__end_entry64_text = .;
	
    __begin_hwint_text = .;
	.hwint.text ALIGN(4) : AT(ADDR(.hwint.text)-VIRTUAL_ADDRESS){ *(.hwint.text) }
	__end_hwint_text = .;
	
	__begin_sirq_text = .;
	.sirq.text ALIGN(4) : AT(ADDR(.sirq.text)-VIRTUAL_ADDRESS) { *(.sirq.sirq) }
	__end_sirq_text = .;
	
	__begin_scall_text = .;
	.scall.text ALIGN(4) : AT(ADDR(.scall.text)-VIRTUAL_ADDRESS) {*(.scall.text) }
	__end_scall_text = .;
	
	__begin_sched_text = .;
	.sched.text ALIGN(4) : AT(ADDR(.sched.text)-VIRTUAL_ADDRESS) { *(.sched.text) }
	__end_sched_text = .;
	
	__begin_fs_text = .;
	.fs.text ALIGN(4) : AT(ADDR(.fs.text)-VIRTUAL_ADDRESS) { *(.fs.text) }
	__end_fs_text = .;

	__begin_drive_text = .;
	.drive.text ALIGN(4) : AT(ADDR(.drive.text)-VIRTUAL_ADDRESS) { *(.drive.text)}
	__end_drive_text = .;
	
	__begin_text = .;
        .text  ALIGN(4) : AT(ADDR(.text)-VIRTUAL_ADDRESS) { *(.text) }
	__end_text = .;
	
	__begin_spinlock_text = .;
	.spinlock.text ALIGN(4) : AT(ADDR(.spinlock.text)-VIRTUAL_ADDRESS) { *(.spinlock.text) }
	__end_spinlock_text = .;
	
	__begin_init_text = .;
	.init.text ALIGN(4) : AT(ADDR(.init.text)-VIRTUAL_ADDRESS) { *(.init.text)}
	__end_init_text = .;
	
	__begin_driver_entry_text = .;
	.drventry.text ALIGN(4) : AT(ADDR(.drventry.text)-VIRTUAL_ADDRESS) { *(.drventry.text)}
	__end_driver_entry_text = .;

	__begin_data = .;
        .data ALIGN(4) : AT(ADDR(.data)-VIRTUAL_ADDRESS) { *(.data) *(.bss) }
 	__end_data = .;
	
	__begin_rodata = .;
	.rodata ALIGN(4) : AT(ADDR(.rodata)-VIRTUAL_ADDRESS) { *(.rodata) *(.rodata.*) }
	__end_rodata = .;

	__begin_kstrtab = .;
	.kstrtab ALIGN(4) : AT(ADDR(.kstrtab)-VIRTUAL_ADDRESS) { *(.kstrtab) }
	__end_kstrtab = .;

	__begin_apboot_text = .;
	.apboot.text ALIGN(4) : AT(ADDR(.apboot.text)-VIRTUAL_ADDRESS) { *(.apboot.text) }
	__end_apboot_text = .;	
	
	__begin_apboot_data = .;
	.apboot.data ALIGN(4) : AT(ADDR(.apboot.data)-VIRTUAL_ADDRESS) { *(.apboot.data) }
	__end_apboot_data = .;	

	__begin_kmodsattrb_data = .;
	.kmodsattrbdata ALIGN(4) : AT(ADDR(.kmodsattrbdata)-VIRTUAL_ADDRESS) { *(.kmodsattrbdata) }
	__end_kmodsattrb_data = .;

	/*__begin_bss = .;
      .bss ALIGN(4) : AT(ADDR(.bss)-VIRTUAL_ADDRESS) { *(.bss) }
	__end_bss = .;*/
	
	.krnl.krnend ALIGN(4) : AT(ADDR(.krnl.krnend)-VIRTUAL_ADDRESS) { *(.krnl.krnend) }
	
	__end_kernel = .;
}

#endif
