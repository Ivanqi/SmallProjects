package linker

import "sort"

// 跟input section 一一对应
type MergeableSection struct {
	Parent      *MergedSection
	P2Align     uint8
	Strs        []string
	FragOffsets []uint32 // 排好序的
	Fragments   []*SectionFragment
}

/**
 * @description: 获取Fragment
 * @param {uint32} offset
 * @return {*SectionFragment, uint32} 返回一个SectionFragment，和offset 距离nFragment的差值
 */
func (m *MergeableSection) GetFragment(offset uint32) (*SectionFragment, uint32) {
	// 找到offset 在 m.FragOffsets 的那个位置
	pos := sort.Search(len(m.FragOffsets), func(i int) bool {
		return offset < m.FragOffsets[i]
	})

	if pos == 0 {
		return nil, 0
	}

	idx := pos - 1
	return m.Fragments[idx], offset - m.FragOffsets[idx]
}
