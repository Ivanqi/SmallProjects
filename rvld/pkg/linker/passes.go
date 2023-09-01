package linker

import (
	"math"
	"rvld/pkg/utils"
)

/**
 * @description: 创建内部文件
 * @param {*Context} ctx
 * @return {*}
 */
func CreateInternalFile(ctx *Context) {
	obj := &ObjectFile{}
	ctx.InternalObj = obj
	ctx.Objs = append(ctx.Objs, obj)

	ctx.InternalEsyms = make([]Sym, 1)
	obj.Symbols = append(obj.Symbols, NewSymbol(""))
	obj.FirstGlobal = 1
	obj.IsAlive = true

	obj.ElfSyms = ctx.InternalEsyms
}

/**
 * @description: 找到这个symbols是从哪里来的
 * @param {*Context} ctx
 * @return {*}
 */
func ResolveSymbols(ctx *Context) {
	for _, file := range ctx.Objs {
		file.ResolveSymbols()
	}

	MarkLiveObjects(ctx)

	for _, file := range ctx.Objs {
		if !file.IsAlive {
			// 清空symbol
			file.ClearSymbols()
		}
	}

	// 把非alive 文件清除
	ctx.Objs = utils.RemoveIf[*ObjectFile](ctx.Objs, func(file *ObjectFile) bool {
		return !file.IsAlive
	})
}

/**
 * @description: 标记活跃的Objects
 * @param {*Context} ctx
 * @return {*}
 */
func MarkLiveObjects(ctx *Context) {
	roots := make([]*ObjectFile, 0)
	for _, file := range ctx.Objs {
		if file.IsAlive {
			roots = append(roots, file)
		}
	}

	utils.Assert(len(roots) > 0)

	for len(roots) > 0 {
		file := roots[0]
		if !file.IsAlive {
			continue
		}
		// 如果文件是alive
		file.MarkLiveObjects(func(file *ObjectFile) {
			// 把所依赖的文件添加到了roots中
			roots = append(roots, file)
		})

		roots = roots[1:]
	}
}

/**
 * @description: 初始化Mergeable section。构建fragment
 * @param {*Context} ctx
 * @return {*}
 */
func RegisterSectionPieces(ctx *Context) {
	for _, file := range ctx.Objs {
		file.RegisterSectionPieces()
	}
}

/**
 * @description: 合成section
 * @param {*Context} ctx
 * @return {*}
 */
func CreateSyntheticSections(ctx *Context) {
	// 返回的是一个output chunk
	push := func(chunk Chunker) Chunker {
		ctx.Chunks = append(ctx.Chunks, chunk)
		return chunk
	}

	ctx.Ehdr = push(NewOutputEhdr()).(*OutputEhdr)
	ctx.Shdr = push(NewOutputShdr()).(*OutputShdr)
}

/**
 * @description: 得到文件大小
 * @param {*Context} ctx
 * @return {*}
 */
func SetOutputSectionOffsets(ctx *Context) uint64 {
	fileoff := uint64(0)
	for _, c := range ctx.Chunks {
		fileoff = utils.AlignTo(fileoff, c.GetShdr().AddrAlign)
		c.GetShdr().Offset = fileoff
		fileoff += c.GetShdr().Size
	}

	return fileoff
}

/**
 * @description: 用于填充output section 里的members数组
 * @param {*Context} ctx
 * @return {*}
 */
func BinSections(ctx *Context) {
	group := make([][]*InputSection, len(ctx.OutputSections))
	for _, file := range ctx.Objs {
		for _, isec := range file.Sections {
			if isec == nil || !isec.IsAlive {
				continue
			}

			idx := isec.OutputSection.Idx
			group[idx] = append(group[idx], isec)
		}
	}

	for idx, osec := range ctx.OutputSections {
		osec.Members = group[idx]
	}
}

/**
 * @description: 收集所有output section
 * @param {*Context} ctx
 * @return {*}
 */
func CollectOutputSections(ctx *Context) []Chunker {
	osecs := make([]Chunker, 0)
	for _, osec := range ctx.OutputSections {
		if len(osec.Members) > 0 {
			osecs = append(osecs, osec)
		}
	}

	return osecs
}

/**
 * @description: 计算每个input section 在output section 中的offset
 * @param {*Context} ctx
 * @return {*}
 */
func ComputeSectionSizes(ctx *Context) {
	for _, osec := range ctx.OutputSections {
		offset := uint64(0)
		p2align := int64(0)

		for _, isec := range osec.Members {
			offset = utils.AlignTo(offset, 1<<isec.P2Align)
			isec.Offset = uint32(offset)
			offset += uint64(isec.ShSize)
			p2align = int64(math.Max(float64(p2align), float64(isec.P2Align)))
		}

		osec.Shdr.Size = offset
		osec.Shdr.AddrAlign = 1 << p2align
	}
}
