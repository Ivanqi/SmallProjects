package linker

import "bytes"

/**
 * @description: 魔数检测，看是否为ELF文件的标识
 * @param {[]byte} contents 二进制数组
 * @return {*}
 */
func CheckMagic(contents []byte) bool {
	return bytes.HasPrefix(contents, []byte("\177ELF"))
}

/**
 * @description: 写入魔数
 * @param {[]byte} contents
 * @return {*}
 */
func WriteMagic(contents []byte) {
	// 8 进制的177 换算成 16进制的7f
	copy(contents, "\177ELF")
}
