package linker

import (
	"rvld/pkg/utils"
)

/**
 * @description: 读取链接库
 * @param {*Context} ctx
 * @param {[]string} remaining
 * @return {*}
 */
func ReadInputFiles(ctx *Context, remaining []string) {
	for _, arg := range remaining {
		var ok bool
		// 如果是 -lgcc 、-lc。直接寻找位置
		if arg, ok = utils.RemovePrefix(arg, "-l"); ok {
			// files := FindLibrary(ctx, arg)
			// println(files.Name)
			ReadFile(ctx, FindLibrary(ctx, arg))
		} else {
			// 如果是 *.o 文件就创建新的文件
			ReadFile(ctx, MustNewFile(arg))
		}
	}
}

/**
 * @description: 文件读取
 * @param {*Context} ctx
 * @param {*File} file
 * @return {*}
 */
func ReadFile(ctx *Context, file *File) {
	ft := GetFileType(file.Contents)
	switch ft {
	case FileTypeObject:
		ctx.Objs = append(ctx.Objs, CreateObjectFile(ctx, file, false))
	case FileTypeArchive: // 静态链接库文件
		for _, child := range ReadArchiveMembers(file) {
			utils.Assert(GetFileType(child.Contents) == FileTypeObject) // 确保是object文件
			ctx.Objs = append(ctx.Objs, CreateObjectFile(ctx, child, true))
		}
	default:
		utils.Fatal("unknown file type")
	}
}

/**
 * @description: 生成object 文件
 * @param {*Context} ctx
 * @param {*File} file
 * @param {bool} inLib
 * @return {*}
 */
func CreateObjectFile(ctx *Context, file *File, inLib bool) *ObjectFile {
	CheckFileCompatibility(ctx, file)

	obj := NewObjectFile(file, !inLib)
	obj.Parse(ctx)
	return obj
}
