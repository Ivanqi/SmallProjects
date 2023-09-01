package linker

import (
	"debug/elf"
	"fmt"
	"rvld/pkg/utils"
)

type InputFile struct {
	File         *File
	ElfSections  []Shdr // 存储整个section header table
	ElfSyms      []Sym
	FirstGlobal  int // 记录第一个Global的Symbol 的位置
	ShStrtab     []byte
	SymbolStrtab []byte
	IsAlive      bool      // 用于标识是否需要保留的链接库文件
	Symbols      []*Symbol // 存储所有symbol
	LocalSymbols []Symbol  // 存储所有Local symbol
}

func NewInputFile(file *File) InputFile {
	f := InputFile{File: file}
	// 文件内容大小必须大于elf header 大小
	if len(file.Contents) < EhdrSize {
		utils.Fatal("file too small")
	}

	// 检测魔数
	if !CheckMagic(file.Contents) {
		utils.Fatal("not an ELF file")
	}

	// 读取Ehdr结构体大小的内容，最后返回Ehdr结构体
	ehdr := utils.Read[Ehdr](file.Contents)
	// file.Contents[ehdr.ShOff:] 得到的是 section header table
	contents := file.Contents[ehdr.ShOff:]
	// 通过对应的section header table就可以读取section，并返回一个Shdr 结构体
	shdr := utils.Read[Shdr](contents)

	// ehdr.ShNum 一共会有多少section header
	numSections := int64(ehdr.ShNum)
	if numSections == 0 {
		numSections = int64(shdr.Size)
	}

	f.ElfSections = []Shdr{shdr}
	// 循环读取section
	for numSections > 1 {
		contents = contents[ShdrSize:]
		f.ElfSections = append(f.ElfSections, utils.Read[Shdr](contents))
		numSections--
	}

	shstrndx := int64(ehdr.ShStrndx)
	// 如果等于65535，就需要找真正的值
	if ehdr.ShStrndx == uint16(elf.SHN_XINDEX) {
		shstrndx = int64(shdr.Link)
	}

	// 返回该section header 的名字
	f.ShStrtab = f.GetBytesFromIdx(shstrndx)
	// .symtab.strtab.shstrtab.rela.text.data.bss.rodata.comment.note.GNU-stack
	// fmt.Printf("%s\n", f.ShStrtab)

	return f
}

/**
 * @description: 获取section header 的某段区域
 * @param {*Shdr} s
 * @return {*}
 */
func (f *InputFile) GetBytesFromShdr(s *Shdr) []byte {
	end := s.Offset + s.Size
	if uint64(len(f.File.Contents)) < end {
		utils.Fatal(fmt.Sprintf("section header is out of range: %d", s.Offset))
	}
	return f.File.Contents[s.Offset:end]
}

func (f *InputFile) GetBytesFromIdx(idx int64) []byte {
	return f.GetBytesFromShdr(&f.ElfSections[idx])
}

/**
 * @description: 填充symbol到ElfSyms
 * @param {*Shdr} s
 * @return {*}
 */
func (f *InputFile) FillUpElfSyms(s *Shdr) {
	bs := f.GetBytesFromShdr(s)
	// 获取所有symbol数量
	f.ElfSyms = utils.ReadSlice[Sym](bs, SymSize)
}

/**
 * @description: 按类型寻找对应的 secton
 * @param {uint32} ty 类型
 * @return {*}
 */
func (f *InputFile) FindSection(ty uint32) *Shdr {
	for i := 0; i < len(f.ElfSections); i++ {
		shdr := &f.ElfSections[i]
		if shdr.Type == ty {
			return shdr
		}
	}

	return nil
}

/**
 * @description: 读取 inputfile 里的 ehdr
 * @return {*}
 */
func (f *InputFile) GetEhdr() Ehdr {
	return utils.Read[Ehdr](f.File.Contents)
}
