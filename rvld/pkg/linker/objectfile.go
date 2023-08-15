package linker

import (
	"debug/elf"
)

type ObjectFile struct {
	InputFile

	SymtabSec *Shdr
}

func NewObjectFile(file *File) *ObjectFile {
	o := &ObjectFile{InputFile: NewInputFile(file)}
	return o
}

/**
 * @description: 获取symbol 数据
 * @return {*}
 */
func (o *ObjectFile) Parse() {
	//  找到 symbol table 类型的section
	o.SymtabSec = o.FindSection(uint32(elf.SHT_SYMTAB))
	if o.SymtabSec != nil {
		// 一个object 文件里有很多符号(symbol), 符号分Local(位于symbol table前边)和Global(位于symbol table后边)
		// info里记录第一个FirstGlobal
		o.FirstGlobal = int64(o.SymtabSec.Info)
		o.FillUpElfSyms(o.SymtabSec)
		// o.SymtabSec.Link 指向的section header 是一个 针对symbol 的 strtab
		o.SymbolStrtab = o.GetBytesFromIdx(int64(o.SymtabSec.Link))
	}
}
