package linker

import (
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
	ctx.Ehdr = NewOutputEhdr()
	ctx.Chunks = append(ctx.Chunks, ctx.Ehdr)
}

/**
 * @description: 得到文件的大小
 * @param {*Context} ctx
 * @return {*}
 */
func GetFileSize(ctx *Context) uint64 {
	fileoff := uint64(0)
	for _, c := range ctx.Chunks {
		fileoff = utils.AlignTo(fileoff, c.GetShdr().AddrAlign)
		fileoff += c.GetShdr().Size
	}

	return fileoff
}
