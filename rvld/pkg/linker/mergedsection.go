package linker

import (
	"debug/elf"
	"rvld/pkg/utils"
	"sort"
)

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

/**
 * @description: 初始化offset
 * @return {*}
 */
func (m *MergedSection) AssignOffsets() {
	var fragments []struct {
		Key string
		Val *SectionFragment
	}

	for key := range m.Map {
		fragments = append(fragments, struct {
			Key string
			Val *SectionFragment
		}{Key: key, Val: m.Map[key]})
	}

	// 对fragments 进行排序
	sort.SliceStable(fragments, func(i, j int) bool {
		x := fragments[i]
		y := fragments[j]
		if x.Val.P2Align != y.Val.P2Align {
			return x.Val.P2Align < y.Val.P2Align
		}

		if len(x.Key) != len(y.Key) {
			return len(x.Key) < len(y.Key)
		}

		return x.Key < y.Key
	})

	offset := uint64(0)
	p2align := uint64(0) // 取fragments中最大的
	for _, frag := range fragments {
		offset = utils.AlignTo(offset, 1<<frag.Val.P2Align)
		frag.Val.Offset = uint32(offset)
		offset += uint64(len(frag.Key))
		if p2align < uint64(frag.Val.P2Align) {
			p2align = uint64(frag.Val.P2Align)
		}
	}

	m.Shdr.Size = utils.AlignTo(offset, 1<<p2align)
	m.Shdr.AddrAlign = 1 << p2align
}

/**
 * @description: 把 fragment  拷贝到buf中
 * @param {*Context} ctx
 * @return {*}
 */
func (m *MergedSection) CopyBuf(ctx *Context) {
	buf := ctx.Buf[m.Shdr.Offset:]
	for key := range m.Map {
		if frag, ok := m.Map[key]; ok {
			// key: frag 实际的数据
			copy(buf[frag.Offset:], key)
		}
	}
}
