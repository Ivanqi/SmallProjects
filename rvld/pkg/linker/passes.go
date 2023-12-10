package linker

import (
	"debug/elf"
	"math"
	"rvld/pkg/utils"
	"sort"
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
	addr := IMAGE_BASE

	for _, chunk := range ctx.Chunks {
		if chunk.GetShdr().Flags&uint64(elf.SHF_ALLOC) == 0 {
			continue
		}

		// 算出虚拟地址
		addr = utils.AlignTo(addr, chunk.GetShdr().AddrAlign)
		chunk.GetShdr().Addr = addr

		if !isTbss(chunk) {
			addr += chunk.GetShdr().Size
		}
	}

	i := 0
	first := ctx.Chunks[0]
	for {
		shdr := ctx.Chunks[i].GetShdr()
		// 算偏移量
		shdr.Offset = shdr.Addr - first.GetShdr().Addr
		i++

		// i 超了chunks 长度就跳过
		if i >= len(ctx.Chunks) || ctx.Chunks[i].GetShdr().Flags&uint64(elf.SHF_ALLOC) == 0 {
			break
		}
	}

	lastShdr := ctx.Chunks[i-1].GetShdr()
	fileoff := lastShdr.Offset + lastShdr.Size

	// 如果还有剩余的 non-alloc section，继续计算
	for ; i < len(ctx.Chunks); i++ {
		shdr := ctx.Chunks[i].GetShdr()
		fileoff = utils.AlignTo(fileoff, shdr.AddrAlign)
		shdr.Offset = fileoff
		fileoff += shdr.Size
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
 * @description: 收集所有output section 和 merged section
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

	for _, osec := range ctx.MergedSections {
		if osec.Shdr.Size > 0 {
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

/**
* @description: outputsection 排序
	为什么要给outputsection 排序. 因为一个program segment 对应多个section
	为了可以缩小空间需要把多个section 合并起来
	排序的条件
		1. EHDR 为0
		2. PHDR 为1
		3. .note sections 2
		4. alloc sections 3
		5. non-alloc sections MAX_INT32 - 1
		6. SHDR 为MAX_INT32
* @param {*Context} ctx
* @return {*}
*/
func SortOutputSections(ctx *Context) {
	// 接收一个chunk
	rank := func(chunk Chunker) int32 {
		typ := chunk.GetShdr().Type
		flags := chunk.GetShdr().Flags

		if flags&uint64(elf.SHF_ALLOC) == 0 {
			// non-alloc sections
			return math.MaxInt32 - 1
		}

		if chunk == ctx.Shdr {
			return math.MaxInt32
		}

		if chunk == ctx.Ehdr {
			return 0
		}

		if typ == uint32(elf.SHT_NOTE) {
			return 2
		}

		b2i := func(b bool) int {
			if b {
				return 1
			}
			return 0
		}

		writeable := b2i(flags&uint64(elf.SHF_WRITE) != 0)   // writeable
		notExec := b2i(flags&uint64(elf.SHF_EXECINSTR) == 0) // 不行可执行
		notTls := b2i(flags&uint64(elf.SHF_TLS) == 0)        // thread loacl storage 线程独享数据
		isBss := b2i(typ == uint32(elf.SHT_NOBITS))          // bss 段

		return int32(writeable<<7 | notExec<<6 | notTls<<5 | isBss<<4)
	}

	sort.SliceStable(ctx.Chunks, func(i, j int) bool {
		return rank(ctx.Chunks[i]) < rank(ctx.Chunks[j])
	})
}

/**
 * @description: 计算merged section offset
 * @param {*Context} ctx
 * @return {*}
 */
func ComputeMergedSectionSizes(ctx *Context) {
	for _, osec := range ctx.MergedSections {
		osec.AssignOffsets()
	}
}

/**
 * @description: 判断是不是一个tbss段
 * @param {Chunker} chunk
 * @return {*}
 */
func isTbss(chunk Chunker) bool {
	shdr := chunk.GetShdr()
	return shdr.Type == uint32(elf.SHT_NOBITS) && shdr.Flags&uint64(elf.SHF_TLS) != 0
}
