package linker

import (
	"rvld/pkg/utils"
)

func ReadInputFiles(ctx *Context, remaining []string) {
	for _, arg := range remaining {
		var ok bool
		if arg, ok = utils.RemovePrefix(arg, "-l"); ok {
			// ReadFile
		}
	}
}

func ReadFile(ctx *Context, file *File) {
	ft := GetFileType(file.Contents)
	switch ft {
	case FileTypeObject:
		// ctx.Objs = append(ctx.Objs, CreateObjectFile(ctx, file, false))
	case FileTypeArchive:
		// for _, child := range ReadArchiveMembers(file) {
		// 	utils.Assert(GetFileType(child.Contents) == FileTypeObject)
		// 	ext.Objs = append(ctx.Objs, CreateObjectFile(ctx, child, true))
		// }
	default:
		utils.Fatal("unkown file type")
	}
}

// func CreateObjectFile(ctx *Context, file *File, inLib bool) *ObjectFile {
// 	CheckFileCompatibility(ctx, file)

// 	obj := NewObjectFile(file, !inLib)
// 	obj.Parse(ctx)
// 	return obj
// }
