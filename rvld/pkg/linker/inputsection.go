package linker

import (
	"debug/elf"
	"math/bits"
	"rvld/pkg/utils"
)

// 表示读入的section
type InputSection struct {
	File     *ObjectFile // section来自的文件
	Contents []byte      // sectiion的数据
	Shndx    uint32      // 在Shdr 的下标值
	ShSize   uint32      // input section 所属的section 的 size
	IsAlive  bool        // 表示这个input section 是不是放到最终的可执行文件中
	P2Align  uint8       // power of 2. 对齐方式

	Offset        uint32
	OutputSection *OutputSection // 记录属于那个output section
}

/**
 * @description: 初始化
 * @param {*ObjectFile} file
 * @param {uint32} shndx
 * @return {*}
 */
func NewInputSection(ctx *Context, name string, file *ObjectFile, shndx uint32) *InputSection {
	s := &InputSection{
		File:    file,
		Shndx:   shndx,
		IsAlive: true,
	}

	shdr := s.Shdr()
	s.Contents = file.File.Contents[shdr.Offset : shdr.Offset+shdr.Size]

	// 判断文件有没有被压缩。0，表示没有被压缩
	utils.Assert(shdr.Flags&uint64(elf.SHF_COMPRESSED) == 0)
	s.ShSize = uint32(shdr.Size)

	//power of 2. 按2次幂对齐
	toP2Align := func(align uint64) uint8 {
		if align == 0 {
			return 0
		}
		return uint8(bits.TrailingZeros64(align))
	}

	s.P2Align = toP2Align(shdr.AddrAlign)

	s.OutputSection = GetOutputSection(ctx, name, uint64(shdr.Type), shdr.Flags)

	return s
}

/**
 * @description: 得到某个section
 * @return {*}
 */
func (i *InputSection) Shdr() *Shdr {
	utils.Assert(i.Shndx < uint32(len(i.File.ElfSections)))
	return &i.File.ElfSections[i.Shndx]
}

/**
 * @description: 得到section的名字
 * @return {*}
 */
func (i *InputSection) Name() string {
	return ElfGetName(i.File.ShStrtab, i.Shdr().Name)
}

/**
 * @description: 把内容写到buf中
 * @param {[]byte} buf
 * @return {*}
 */
func (i *InputSection) WriteTo(buf []byte) {
	// 判断section header type 等于 SHT_NOBITS(.bss段)不写入
	if i.Shdr().Type == uint32(elf.SHT_NOBITS) || i.ShSize == 0 {
		return
	}

	i.CopyContents(buf)
}

/**
 * @description: 把content的内容拷贝到buf中
 * @param {[]byte} buf
 * @return {*}
 */
func (i *InputSection) CopyContents(buf []byte) {
	copy(buf, i.Contents)
}
