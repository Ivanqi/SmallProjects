package linker

import "debug/elf"

// 把多个mergeablesection合并成一个merged section。最终写入可执行文件中
type MergedSection struct {
	Chunk
	Map map[string]*SectionFragment
}

func NewMergedSection(name string, flags uint64, typ uint32) *MergedSection {
	m := &MergedSection{
		Chunk: NewChunk(),
		Map:   make(map[string]*SectionFragment),
	}

	m.Name = name
	m.Shdr.Flags = flags
	m.Shdr.Type = typ

	return m
}

/**
 * @description: 获取 MergedSection 实例。为了建立 input section 和output section 的对应关系
	1. name 必须是相同的
 * @param {*Context} ctx
 * @param {string} name
 * @param {uint32} typ
 * @param {uint64} flags
 * @return {*}
*/
func GetMergedSectionInstance(ctx *Context, name string, typ uint32, flags uint64) *MergedSection {
	name = GetOutputName(name, flags)
	// 为flag 清除多余的位(非组、非合并、非字符串字面量、非加密)，为不让这些值干扰判断
	flags = flags & ^uint64(elf.SHF_GROUP) & ^uint64(elf.SHF_MERGE) &
		^uint64(elf.SHF_STRINGS) & ^uint64(elf.SHF_COMPRESSED)

	find := func() *MergedSection {
		for _, osec := range ctx.MergedSections {
			if name == osec.Name && flags == osec.Shdr.Flags && typ == osec.Shdr.Type {
				return osec
			}
		}

		return nil
	}

	if osec := find(); osec != nil {
		return osec
	}

	osec := NewMergedSection(name, flags, typ)
	ctx.MergedSections = append(ctx.MergedSections, osec)
	return osec
}

/**
 * @description: 往mergedsection 插入新的SectionFragment
 * @param {string} key
 * @param {uint32} p2align
 * @return {*}
 */
func (m *MergedSection) Insert(key string, p2align uint32) *SectionFragment {
	frag, ok := m.Map[key]
	if !ok {
		frag = NewSectionFragment(m)
		m.Map[key] = frag
	}

	if frag.P2Align < p2align {
		frag.P2Align = p2align
	}

	return frag
}
