/*
 * @Author: ivanqi
 * @Date: 2023-08-18 23:47:44
 * @LastEditors: ivanqi
 * @LastEditTime: 2023-08-31 19:32:55
 * @Description:
 */
package linker

import (
	"rvld/pkg/utils"
)

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
		file.MarkLiveObjects(ctx, func(file *ObjectFile) {
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
