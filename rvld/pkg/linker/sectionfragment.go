package linker

import "math"

// 表示一个小的数据块
type SectionFragment struct {
	OutputSection *MergedSection // 双向关联
	Offset        uint32         // section 里的offset
	P2Align       uint32         // 2次幂对齐
	IsAlive       bool
}

/**
 * @description: 初始化
 * @param {*MergedSection} m
 * @return {*}
 */
func NewSectionFragment(m *MergedSection) *SectionFragment {
	return &SectionFragment{
		OutputSection: m,
		Offset:        math.MaxUint32,
	}
}
