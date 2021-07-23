###################################################################
#		主控自动化编译配置文件 Makefile			                      #
###################################################################

MAKEFLAGS = -sR

MKDIR = mkdir
RMDIR = rmdir
CP = cp
CD = cd
DD = dd
RM = rm

LKIMG = ./lmoskrlimg -m k

# qemu-system-x86_64 使用 qemu-system-x86 来启动 x86 架构的虚拟机
VM = qemu-system-x86_64

# bochs是一个x86平台模拟器
DBUGVM = bochs -q

# qemu-img是创建、转换、修改磁盘映像的工具
IMGTOVDI = qemu-img convert -f raw -O vdi
IMGTOVMDK = qemu-img convert -f raw -O vmdk
MAKE = make

X86BARD = -f ./Makefile.x86