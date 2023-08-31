package linker

import (
	"debug/elf"
	"strings"
)

var prefixes = []string{
	".text.", ".data.rel.ro.", ".data.", ".rodata.", ".bss.rel.ro.", ".bss.",
	".init_array.", ".fini_array.", ".tbss.", ".tdata.", ".gcc_except_table.",
	".ctors.", ".dtors.",
}

/**
 * @description: 把一个input name 处理成output name
 * @param {string} name
 * @param {uint64} flags
 * @return {*}
 */
func GetOutputName(name string, flags uint64) string {
	// 判断section 是不是readonly 并且 是合并(merged)的section
	if (name == ".rodata" || strings.HasPrefix(name, ".rodata.")) && flags&uint64(elf.SHF_MERGE) != 0 {
		// merged 分两种.有 SHF_STRINGS，就是字符串字面量，不是就是一些常量(const)
		if flags&uint64(elf.SHF_STRINGS) != 0 {
			return ".rodata.str"
		} else {
			return ".rodata.cst"
		}
	}

	for _, prefix := range prefixes {
		stem := prefix[:len(prefix)-1]
		if name == stem || strings.HasPrefix(name, prefix) {
			return stem
		}
	}

	return name
}
