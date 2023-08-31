package linker

import "rvld/pkg/utils"

type Symbol struct {
	File   *ObjectFile
	Name   string
	Value  uint64 //sym struct 的Val属性
	SymIdx int    //在 inputFile ElfSyms 中的下标

	// 某一个symbol可能属于inputsection or SectionFragment
	InputSection    *InputSection // 属于/不属于某个inputsectioin
	SectionFragment *SectionFragment
}

func NewSymbol(name string) *Symbol {
	s := &Symbol{Name: name}
	return s
}

func (s *Symbol) SetInputSection(isec *InputSection) {
	s.InputSection = isec
	s.SectionFragment = nil
}

func (s *Symbol) SetSectionFragment(frag *SectionFragment) {
	s.InputSection = nil
	s.SectionFragment = frag
}

/**
 * @description: 获取 symbol 的名字
	1. 从context结构体的SymbolMap中获取，如果没有就创建新建一个symbol
	2. 并重新存储在SymbolMap中
 * @param {*Context} ctx
 * @param {string} name
 * @return {*}
*/
func GetSymbolByName(ctx *Context, name string) *Symbol {
	if sym, ok := ctx.SymbolMap[name]; ok {
		return sym
	}

	ctx.SymbolMap[name] = NewSymbol(name)
	return ctx.SymbolMap[name]
}

/**
 * @description: 获取section中获取symbol
	1. 一个 elf section 对应一个symbol
 * @return {*}
*/
func (s *Symbol) ElfSym() *Sym {
	utils.Assert(s.SymIdx < len(s.File.ElfSyms))
	return &s.File.ElfSyms[s.SymIdx]
}

/**
 * @description: 对Symbol 的属性置空
 * @return {*}
 */
func (s *Symbol) Clear() {
	s.File = nil
	s.InputSection = nil
	s.SymIdx = -1
}
