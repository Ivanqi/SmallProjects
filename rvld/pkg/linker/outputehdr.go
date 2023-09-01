package linker

import (
	"bytes"
	"debug/elf"
	"encoding/binary"
	"rvld/pkg/utils"
)

type OutputEhdr struct {
	Chunk
}

func NewOutputEhdr() *OutputEhdr {
	return &OutputEhdr{Chunk{
		Shdr: Shdr{
			Flags:     uint64(elf.SHF_ALLOC), // 执行期时创建内存
			Size:      uint64(EhdrSize),
			AddrAlign: 8,
		},
	}}
}

func getEntryAddr(ctx *Context) uint64 {
	for _, osec := range ctx.OutputSections {
		// 读到text section 的时候，返回地址
		if osec.Name == ".text" {
			return osec.Shdr.Addr
		}
	}

	return 0
}

func getFlags(ctx *Context) uint32 {
	utils.Assert(len(ctx.Objs) > 0)
	flags := ctx.Objs[0].GetEhdr().Flags
	for _, obj := range ctx.Objs[1:] {
		// 内部object 需要跳过
		if obj == ctx.InternalObj {
			continue
		}

		// RVC RISC-V Compressed Extension 用了这个扩展，就会有这个标识。16位
		if obj.GetEhdr().Flags&EF_RISCV_RVC != 0 {
			flags |= EF_RISCV_RVC
			break
		}
	}

	return flags
}

func (o *OutputEhdr) CopyBuf(ctx *Context) {
	// 生成elf header
	ehdr := &Ehdr{}
	WriteMagic(ehdr.Ident[:])
	ehdr.Ident[elf.EI_CLASS] = uint8(elf.ELFCLASS64)   // elf_class.表示是多少位的
	ehdr.Ident[elf.EI_DATA] = uint8(elf.ELFDATA2LSB)   // 大小端模式。小端模式
	ehdr.Ident[elf.EI_VERSION] = uint8(elf.EV_CURRENT) // version.
	ehdr.Ident[elf.EI_OSABI] = 0                       // 操作系统类型
	ehdr.Ident[elf.EI_ABIVERSION] = 0                  // ABI
	ehdr.Type = uint16(elf.ET_EXEC)                    // 可执行文件
	ehdr.Machine = uint16(elf.EM_RISCV)                // riscv机器架构
	ehdr.Version = uint32(elf.EV_CURRENT)              // version
	ehdr.Entry = getEntryAddr(ctx)                     // 第一条指令的虚拟地址

	ehdr.ShOff = ctx.Shdr.Shdr.Offset // section header offset
	ehdr.Flags = getFlags(ctx)
	ehdr.EhSize = uint16(EhdrSize)
	ehdr.PhEntSize = uint16(PhdrSize)

	ehdr.ShEntSize = uint16(ShdrSize)
	ehdr.ShNum = uint16(ctx.Shdr.Shdr.Size) / uint16(ShdrSize)

	buf := &bytes.Buffer{}
	err := binary.Write(buf, binary.LittleEndian, ehdr)
	utils.MustNo(err)
	copy(ctx.Buf[o.Shdr.Offset:], buf.Bytes())
}
