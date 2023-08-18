package linker

import (
	"debug/elf"
	"rvld/pkg/utils"
)

type ObjectFile struct {
	InputFile
	SymtabSec      *Shdr
	SymtabShndxSec []uint32
	Sections       []*InputSection
}

func NewObjectFile(file *File, isAlive bool) *ObjectFile {
	o := &ObjectFile{InputFile: NewInputFile(file)}
	o.IsAlive = isAlive
	return o
}

/**
 * @description: 获取symbol 数据
 * @return {*}
 */
func (o *ObjectFile) Parse(ctx *Context) {
	//  找到 symbol table 类型的section
	o.SymtabSec = o.FindSection(uint32(elf.SHT_SYMTAB))
	if o.SymtabSec != nil {
		// 一个object 文件里有很多符号(symbol), 符号分Local(位于symbol table前边)和Global(位于symbol table后边)
		// info里记录第一个FirstGlobal
		o.FirstGlobal = int(o.SymtabSec.Info)
		o.FillUpElfSyms(o.SymtabSec)
		// o.SymtabSec.Link 指向的section header 是一个 针对symbol 的 strtab
		o.SymbolStrtab = o.GetBytesFromIdx(int64(o.SymtabSec.Link))
	}

	o.InitializeSections()
	o.InitializeSymbols(ctx)
}

/**
 * @description: 初始化sections
 * @return {*}
 */
func (o *ObjectFile) InitializeSections() {
	o.Sections = make([]*InputSection, len(o.ElfSections))
	for i := 0; i < len(o.ElfSections); i++ {
		shdr := &o.ElfSections[i]
		switch elf.SectionType(shdr.Type) {
		case elf.SHT_GROUP, elf.SHT_SYMTAB, elf.SHT_STRTAB, elf.SHT_REL, elf.SHT_RELA, elf.SHT_NULL:
			// 跳过。因为可能是些辅助性信息
			break
		case elf.SHT_SYMTAB_SHNDX: // 当sym shndx 过大时候，需要来这里找
			o.FillUpSymtabShndxSec(shdr)
		default:
			o.Sections[i] = NewInputSection(o, uint32(i))
		}
	}
}

/**
 * @description: 读取SymtabShndxSec
 * @param {*Shdr} s
 * @return {*}
 */
func (o *ObjectFile) FillUpSymtabShndxSec(s *Shdr) {
	bs := o.GetBytesFromShdr(s)
	nums := len(bs) / 4

	for nums > 0 {
		o.SymtabShndxSec = append(o.SymtabShndxSec, utils.Read[uint32](bs))
		bs = bs[4:]
		nums--
	}
}

/**
 * @description: 初始化symbols
 * @param {*Context} ctx
 * @return {*}
 */
func (o *ObjectFile) InitializeSymbols(ctx *Context) {
	if o.SymtabSec == nil {
		return
	}

	// 创建 LocalSymbols数组。symbol 都是按 Local -> Global 排序的。FirstGlobal 前的都是Local的
	o.LocalSymbols = make([]Symbol, o.FirstGlobal)
	for i := 0; i < len(o.LocalSymbols); i++ {
		o.LocalSymbols[i] = *NewSymbol("")
	}

	// 第0个symbol是特殊的
	o.LocalSymbols[0].File = o

	for i := 1; i < len(o.LocalSymbols); i++ {
		esym := &o.ElfSyms[i]
		sym := &o.LocalSymbols[i]
		sym.Name = ElfGetName(o.SymbolStrtab, esym.Name)
		sym.File = o
		sym.Value = esym.Val
		sym.SymIdx = i

		if !esym.IsAbs() {
			sym.SetInputSection(o.Sections[o.GetShndx(esym, i)])
		}
	}

	o.Symbols = make([]*Symbol, len(o.ElfSyms))
	for i := 0; i < len(o.LocalSymbols); i++ {
		o.Symbols[i] = &o.LocalSymbols[i]
	}

	for i := len(o.LocalSymbols); i < len(o.ElfSyms); i++ {
		esym := &o.ElfSyms[i]
		name := ElfGetName(o.SymbolStrtab, esym.Name)
		o.Symbols[i] = GetSymbolByName(ctx, name)
	}
}

/**
 * @description: 获取 symbol 的 shndex
 * @param {*Sym} esym
 * @param {int} idx
 * @return {*}
 */
func (o *ObjectFile) GetShndx(esym *Sym, idx int) int64 {
	utils.Assert(idx >= 0 && idx < len(o.ElfSyms))

	// 如果symbol Shndx 过大就使用 SymtabShndxSec
	if esym.Shndx == uint16(elf.SHN_XINDEX) {
		return int64(o.SymtabShndxSec[idx])
	}

	return int64(esym.Shndx)
}

/**
 * @description: 找到这个symbols是从哪里来的
 * @return {*}
 */
func (o *ObjectFile) ResolveSymbols() {
	// 只找Global的
	for i := o.FirstGlobal; i < len(o.ElfSyms); i++ {
		sym := o.Symbols[i]
		esym := &o.ElfSyms[i]

		// 未定义的symbol的跳过
		if esym.IsUndef() {
			continue
		}

		var isec *InputSection
		if !esym.IsAbs() {
			// 通过symbol 找到对应的section
			isec = o.GetSection(esym, i)
			if isec == nil {
				continue
			}
		}

		// 如果sym在某个文件中，就不做处理了
		if sym.File == nil {
			sym.File = o
			sym.SetInputSection(isec)
			sym.Value = esym.Val
			sym.SymIdx = i
		}
	}
}

/**
 * @description: 根据一个symbol 拿到 对应的section
 * @param {*Sym} esym
 * @param {int} idx
 * @return {*}
 */
func (o *ObjectFile) GetSection(esym *Sym, idx int) *InputSection {
	return o.Sections[o.GetShndx(esym, idx)]
}

/**
 * @description: 标记活跃的Objects
 * @param {*} ObjectFile
 * @return {*}
 */
func (o *ObjectFile) MarkLiveObjects(ctx *Context, feeder func(*ObjectFile)) {
	// 判断ObjectFile 是否为活跃
	utils.Assert(o.IsAlive)

	// 遍历所有Global
	for i := o.FirstGlobal; i < len(o.ElfSyms); i++ {
		sym := o.Symbols[i]
		esym := &o.ElfSyms[i]

		// sym不属于任何文件
		if sym.File == nil {
			continue
		}

		// sym 为未定义，且sym是未活跃。就必须标记为活跃
		if esym.IsUndef() && !sym.File.IsAlive {
			sym.File.IsAlive = true
			feeder(sym.File)
		}
	}
}

/**
 * @description: 清空symbol
 * @return {*}
 */
func (o *ObjectFile) ClearSymbols() {
	for _, sym := range o.Symbols[o.FirstGlobal:] {
		if sym.File == o {
			sym.Clear()
		}
	}
}
