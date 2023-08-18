package linker

import "rvld/pkg/utils"

/**
 * @description: arch 文件读取
 * @param {*File} file
 * @return {*}
 */
func ReadArchiveMembers(file *File) []*File {
	utils.Assert(GetFileType(file.Contents) == FileTypeArchive)

	pos := 8 // 前8个字节是标识符，需要跳过
	var strTab []byte
	var files []*File

	for len(file.Contents)-pos > 1 {
		// arch 文件以2字节对齐
		if pos%2 == 1 {
			pos++
		}

		// 读取arch的数据
		hdr := utils.Read[ArHdr](file.Contents[pos:])
		// 真正数据的开始位置
		dataStart := pos + ArHdrSize
		pos = dataStart + hdr.GetSize()
		// 真正数据的结束位置
		dataEnd := pos
		contents := file.Contents[dataStart:dataEnd]

		if hdr.IsSymtab() {
			continue
		} else if hdr.IsStrtab() {
			strTab = contents
			continue
		}

		files = append(files, &File{
			Name:     hdr.ReadName(strTab),
			Contents: contents,
			Parent:   file,
		})
	}

	return files
}
