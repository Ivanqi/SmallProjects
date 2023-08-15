package linker

import (
	"bytes"
	"debug/elf"
	"rvld/pkg/utils"
)

type FileType = uint8

const (
	FileTypeUnknown FileType = iota // 不认识的文件类型
	FileTypeEmpty   FileType = iota // 空文件
	FileTypeObject  FileType = iota // object 文件
	FileTypeArchive FileType = iota //
)

/**
 * @description: 获取文件类型
 * @param {[]byte} contents
 * @return {*}
 */
func GetFileType(contents []byte) FileType {
	if len(contents) == 0 {
		return FileTypeEmpty
	}

	// 魔数检测，检测是否为elf文件
	if CheckMagic(contents) {
		et := elf.Type(utils.Read[uint16](contents[16:])) // elf type 类型
		switch et {
		case elf.ET_REL: // 可重定位文件类型
			return FileTypeObject
		}
		return FileTypeUnknown
	}

	// 机器标识检测
	if bytes.HasPrefix(contents, []byte("!<arch>\n")) {
		return FileTypeArchive
	}

	return FileTypeUnknown
}

func CheckFileCompatibility(ctx *Context, file *File) {
	mt := GetMachineTypeFromContents(file.Contents)
	if mt != ctx.Args.Emulation {
		utils.Fatal("incompatible file type")
	}
}
