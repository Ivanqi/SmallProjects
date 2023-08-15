package linker

import (
	"debug/elf"
	"rvld/pkg/utils"
)

type MachineType = uint8

const (
	MachineTypeNone    MachineType = iota
	MachineTypeRISCV64 MachineType = iota
)

/**
 * @description: 从二进制数组中获取机器类型
 * @param {[]byte} contents
 * @return {*}
 */
func GetMachineTypeFromContents(contents []byte) MachineType {
	// 获取文件类型
	ft := GetFileType(contents)

	switch ft {
	case FileTypeObject: // 如果是elf文件，那么就是读取elf header的Machine字段
		machine := utils.Read[uint16](contents[18:])
		if machine == uint16(elf.EM_RISCV) { // riscv机器类型
			// https://refspecs.linuxbase.org/elf/gabi4+/ch4.eheader.html#elfid
			class := elf.Class(contents[4]) // 	e_ident[File class]
			switch class {
			case elf.ELFCLASS64: // 支持具有64位体系结构的机器
				return MachineTypeRISCV64
			}
		}
	}

	return MachineTypeNone
}

type MachineTypeStringer struct {
	MachineType
}

func (m MachineTypeStringer) String() string {
	switch m.MachineType {
	case MachineTypeRISCV64:
		return "riscv64"
	}

	utils.Assert(m.MachineType == MachineTypeNone)
	return "none"
}
