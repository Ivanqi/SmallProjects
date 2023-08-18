package linker

import "rvld/pkg/utils"

// 表示读入的section
type InputSection struct {
	File     *ObjectFile // section来自的文件
	Contents []byte      // sectiion的数据
	Shndx    uint32      // 在Shdr 的下标值
}

func NewInputSection(file *ObjectFile, shndx uint32) *InputSection {
	s := &InputSection{File: file, Shndx: shndx}

	shdr := s.Shdr()
	s.Contents = file.File.Contents[shdr.Offset : shdr.Offset+shdr.Size]

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
