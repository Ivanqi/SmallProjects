package linker

import (
	"os"
	"rvld/pkg/utils"
)

type File struct {
	Name     string
	Contents []byte
	Parent   *File // 用于表示来自那个文件
}

func MustNewFile(filename string) *File {
	contents, err := os.ReadFile(filename)
	utils.MustNo(err)
	return &File{
		Name:     filename,
		Contents: contents,
	}
}

/**
 * @description: 打开链接库文件
 * @param {string} filepath
 * @return {*}
 */
func OpenLibrary(filepath string) *File {
	contents, err := os.ReadFile(filepath)
	if err != nil {
		return nil
	}

	return &File{
		Name:     filepath,
		Contents: contents,
	}
}

/**
 * @description: 寻找链接库位置
 * @param {*Context} ctx
 * @param {string} name
 * @return {*}
 */
func FindLibrary(ctx *Context, name string) *File {
	for _, dir := range ctx.Args.LibraryPaths {
		// 例子.gcc处理成 /libgcc.a
		stem := dir + "/lib" + name + ".a"
		if f := OpenLibrary(stem); f != nil {
			return f
		}
	}

	utils.Fatal("library not found")
	return nil
}
