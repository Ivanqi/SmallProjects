package linker

import (
	"bytes"
	"debug/elf"
	"rvld/pkg/utils"
	"strconv"
	"strings"
	"unsafe"
)

const PageSize = 4096

const IMAGE_BASE uint64 = 0x200000
const EF_RISCV_RVC uint32 = 1

const EhdrSize = int(unsafe.Sizeof(Ehdr{}))
const ShdrSize = int(unsafe.Sizeof(Shdr{}))
const PhdrSize = int(unsafe.Sizeof(Phdr{}))
const SymSize = int(unsafe.Sizeof(Sym{}))
const ArHdrSize = int(unsafe.Sizeof(ArHdr{}))
const RelaSize = int(unsafe.Sizeof(Rela{}))

// 描述整个 ELF 文件的重要属性，包括文件类型、目标硬件平台、入口加载地址等信息
// https://refspecs.linuxbase.org/elf/gabi4+/ch4.eheader.html
// https://zhuanlan.zhihu.com/p/73114831
type Ehdr struct {
	Ident     [16]uint8 // ELF文件的标识，包括魔数和其他标识信息
	Type      uint16    // ELF文件的类型，表示文件的用途，如可执行文件、共享库等
	Machine   uint16    // 目标硬件平台的架构类型，如x86、ARM等
	Version   uint32    // ELF文件的版本号
	Entry     uint64    // 程序的入口点，即程序运行时将会执行的第一条指令的地址.第一条指令的虚拟地址
	PhOff     uint64    // 程序头部表的偏移量，即程序头部表在文件中的位置
	ShOff     uint64    // 节头部表的偏移量，即节头部表在文件中的位置
	Flags     uint32    // 与目标平台相关的标志
	EhSize    uint16    // ELF文件头部的大小
	PhEntSize uint16    // 程序头部表项的大小
	PhNum     uint16    // 程序头部表项的数量
	ShEntSize uint16    // 节头部表项的大小
	ShNum     uint16    // 节头部表项的数量。一共有多少个section header,默认不会超过65535
	ShStrndx  uint16    // 包含节名称字符串表的节的索引. 指向的是section header 的下标
}

// 描述 ELF 文件中的节（section），包括节的类型、标志、地址、大小等信息
type Shdr struct {
	Name      uint32 // 节的名称，它是一个索引，指向字符串表中的节名
	Type      uint32 // 节的类型，指示节的内容和用途
	Flags     uint64 // 节的标志，描述了节的属性和特性
	Addr      uint64 // 节在内存中的虚拟地址
	Offset    uint64 // 节在文件中的偏移地址
	Size      uint64 // 节的大小，即占用的字节数
	Link      uint32 // 链接字段，取决于节的类型，可能会被用于不同的目的
	Info      uint32 // 附加信息字段，取决于节的类型，可能会被用于不同的目的。
	AddrAlign uint64 // 节在内存中的对齐方式
	EntSize   uint64 // 节中每个实体（entry）的大小
}

// 描述 ELF 文件中的程序段（program segment），包括段的类型、标志、偏移、虚拟地址、文件大小、内存大小等信息
// https://zhuanlan.zhihu.com/p/36887189
// 执行期使用
type Phdr struct {
	Type     uint32 // 类型
	Flags    uint32
	Offset   uint64 // 表示本段在文件的偏移量
	VAddr    uint64 // 表示本段在内存中起始的虚拟地址
	PAddr    uint64 // 仅用于与物理地址相关的系统中
	FileSize uint64 // 表示本段在文件中的大小
	MemSize  uint64 // 表示本段在内存中的大小
	Align    uint64 // 表示本段在文件和内存中的对齐方式
}

// 描述 ELF 文件中的符号表（symbol table）中的一个符号，包括符号的名称、类型、值、大小等信息
// https://refspecs.linuxbase.org/elf/gabi4+/ch4.symtab.html
type Sym struct {
	Name  uint32 // 符号的名称，使用一个32位的无符号整数表示
	Info  uint8  // 符号的类型和绑定信息，使用一个8位的无符号整数表示
	Other uint8  // 符号的其他属性信息，使用一个8位的无符号整数表示
	Shndx uint16 // 符号的节索引，使用一个16位的无符号整数表示。节索引指示符号所属的节（section）
	Val   uint64 // 符号的值，使用一个64位的无符号整数表示。在可执行文件或共享库中，这个值通常表示符号的地址
	Size  uint64 // 符号的大小，使用一个64位的无符号整数表示
}

/**
* @description: 是否为绝对符号
	1. 判断sym是否为绝对符号
* @return {*}
*/
func (s *Sym) IsAbs() bool {
	return s.Shndx == uint16(elf.SHN_ABS)
}

/**
 * @description: 是否为未定义符号
 * @return {*}
 */
func (s *Sym) IsUndef() bool {
	return s.Shndx == uint16(elf.SHN_UNDEF)
}

func (s *Sym) IsCommon() bool {
	return s.Shndx == uint16(elf.SHN_COMMON)
}

type Rela struct {
	Offset uint64
	Type   uint32
	Sym    uint32
	Addend int64
}

// arch 文件格式
// [!<arch>\n][Section]\n[Section][Section][Section]
// [Section]里的数据: [ArHdr][ 真正的数据         ]
type ArHdr struct {
	Name [16]byte
	Date [12]byte
	Uid  [6]byte
	Gid  [6]byte
	Mode [8]byte
	Size [10]byte // 通过size得到真正数据的长度
	Fmag [2]byte
}

/**
 * @description: arhdr name 的前缀检测
 * @param {string} s
 * @return {*}
 */
func (a *ArHdr) HasPrefix(s string) bool {
	return strings.HasPrefix(string(a.Name[:]), s)
}

/**
 * @description: 检测 arhdr 是不是strtable。
 *  1. strtable 用于保存名字
 * @return {*}
 */
func (a *ArHdr) IsStrtab() bool {
	return a.HasPrefix("// ")
}

/**
 * @description: 检测arhdr 是不是 symtable。是不是object文件
 * @return {*}
 */
func (a *ArHdr) IsSymtab() bool {
	return a.HasPrefix("/ ") || a.HasPrefix("/SYM64/ ")
}

/**
 * @description: 得到 ArHdr size
 * @return {*} 返回数字的 size
 */
func (a *ArHdr) GetSize() int {
	// 把二进制数组转换成字符串
	size, err := strconv.Atoi(strings.TrimSpace(string(a.Size[:])))
	utils.MustNo(err)
	return size
}

/**
 * @description: 读取 arhdr 的name。
 * @param {[]byte} strTab
 * @return {*}
 */
func (a *ArHdr) ReadName(strTab []byte) string {
	// Long filename
	if a.HasPrefix("/") {
		start, err := strconv.Atoi(strings.TrimSpace(string(a.Name[1:])))
		utils.MustNo(err)
		// 例子: "test.o/\n". 名字的文件格式
		end := start + bytes.Index(strTab[start:], []byte("/\n"))
		return string(strTab[start:end])
	}

	// Short filename
	end := bytes.Index(a.Name[:], []byte("/"))
	utils.Assert(end != -1)
	return string(a.Name[:end])
}

/**
 * @description: 获取section 的 name
 * @param {[]byte} strTab
 * @param {uint32} offset
 * @return {*}
 */
func ElfGetName(strTab []byte, offset uint32) string {
	length := uint32(bytes.Index(strTab[offset:], []byte{0}))
	return string(strTab[offset : offset+length])
}
