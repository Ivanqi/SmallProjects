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

	ehdr.EhSize = uint16(EhdrSize)
	ehdr.PhEntSize = uint16(PhdrSize)

	ehdr.ShEntSize = uint16(ShdrSize)

	buf := &bytes.Buffer{}
	err := binary.Write(buf, binary.LittleEndian, ehdr)
	utils.MustNo(err)
	copy(ctx.Buf[o.Shdr.Offset:], buf.Bytes())
}
