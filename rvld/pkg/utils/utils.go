package utils

import (
	"encoding/binary"
	"bytes"
	"fmt"
	"math/bits"
	"os"
	"runtime/debug"
	"strings"
)

/**
 * @description: 函数终止，并输出错误栈
 * @param {any} v 错误信息
 * @return {*}
 */
func Fatal(v any) {
	fmt.Printf("rvld: \033[0;1;31mfatal:\033[0m %v\n", v)
	debug.PrintStack()
	os.Exit(1)
}

/**
 * @description: 必须为错误
 * @param {error} err
 * @return {*}
 */
func MustNo(err error) {
	if err != nil {
		os.Exit(1)
	}
}

/**
 * @description: 自定义断言
 * @param {bool} condition 条件
 * @return {*}
 */
func Assert(condition bool) {
	if !condition {
		Fatal("assert failed")
	}
}

/**
 * @description: 通过泛型方式读取各种二进制数组
 * @return {*} 返回处理后的数据
 */
func Read[T any](data []byte) (val T) {
	reader := bytes.NewReader(data)
	// 小端序读取
	err := binary.Read(reader, binary.LittleEndian, &val)
	MustNo(err)
	return
}

/**
 * @description: 读取一个数组
 * @return {*}
 */
func ReadSlice[T any](data []byte, sz int) []T {
	nums := len(data) / sz
	res := make([]T, 0, nums)
	for nums > 0 {
		res = append(res, Read[T](data))
		data = data[sz:]
		nums--
	}

	return res
}

/**
 * @description: 通过泛型方式写入各种二进制数组
 * @return {*}
 */
func Write[T any](data []byte, e T) {
	buf := &bytes.Buffer{}
	// 将数据转换成byte字节流
	err := binary.Write(buf, binary.LittleEndian, e)
	MustNo(err)
	// 切片拷贝
	copy(data, buf.Bytes())
}

/**
 * @description: 删除前缀
 * @param {*} s 字符串
 * @param {string} prefix 前缀
 * @return {string, bool} 
 */
func RemovePrefix(s, prefix string) (string, bool) {
	// 检测字符串是否以指定的前缀开头
	if strings.HasPrefix(s, prefix) {
		// TrimPrefix() 删掉前缀
		s = strings.TrimPrefix(s, prefix)
		return s, true
	}

	return s, false
}

/**
 * @description: 对不符合的条件的进行删除
 * @param {*} T
 * @return []T
 */
func RemoveIf[T any](elems []T, condition func(T) bool) []T {
	i := 0
	for _, elem := range elems {
		if condition(elem) {
			continue
		}
		elems[i] = elem
		i++
	}

	return elems[:i]
}

/**
 * @description: 全部为零
 * @param {[]byte} bs
 * @return bool
 */
func AllZeros(bs []byte) bool {
	b := byte(0)
	for _, s := range bs {
		b |= s
	}

	return b == 0
}

/**
 * @description: 内存对齐
	1. 如果我们调用AlignTo(10, 4)，表示将数值10按照4的倍数进行对齐
	2. 根据代码的逻辑，对齐操作的结果将会是12，因为12是离10最近的4的倍数
	3. 对齐值必须是2的幂次方，以保证对齐操作的正确性。如果对齐值为0，则直接返回原始数值，表示不进行对齐操作
 * @param {*} val
 * @param {uint64} align
 * @return {*}
 */
func AlignTo(val, align uint64) uint64 {

	// 如果对齐值为0，直接返回原始数值
	if align == 0 {
		return val
	}

	// 对齐操作
	// 首先，将原始数值加上对齐值减一，得到一个与对齐值相差不大的数
	// 然后，通过按位与非操作，将这个数与对齐值减一的二进制反码进行与操作
	// 这样就能将这个数的低位全部置为0，达到对齐的效果
	return (val + align - 1) &^ (align - 1)
}

/**
 * @description: 2的次幂
 * @param {uint64} n
 * @return {*}
 */
func hasSingleBit(n uint64) bool {
	// 如果n只有一个1（即二进制表示中只有一个位为1），那么n-1相当于将这个位取反，然后这两个数进行按位与运算结果必定为0。
	// 假设n的二进制表示为1000，那么n-1的二进制表示为0111，将这两个数进行按位与运算得到的结果为0000。
	return n & (n - 1) == 0;
}

/**
 * @description: 找到大于或等于val的最小的2的幂次方
 * @param {uint64} val
 * @return {*}
 */
func BitCeil(val uint64) uint64 {
	if hasSingleBit(val) {
		return val
	}
	// 如果val不是2的幂次方，那么函数通过调用bits.LeadingZeros64(val)来获取val二进制表示中最高位1之前的0的个数
	// 接下来，函数使用64减去之前得到的结果，并将其作为指数用于计算1左移的位数。这样就得到了大于或等于val的最小的2的幂次方
	return 1 << (64 - bits.LeadingZeros64(val))
}

type Uint interface {
	uint8 | uint16 | uint32 | uint64
}

/**
 * @description: 右移多少位值
 * @return {*}
 */
func Bit[T Uint](val T, pos int) T {
	return (val >> pos) & 1
}


/**
 * @description: 从val 中提取指定范围的位，并返回提取的结果
 * @param {T} val 表示要提取位的整数值
 * @param {T} hi 要提取的位的范围
 * @param {T} lo 要提取的位的范围
 * @return {*}
 */
func Bits[T Uint](val T, hi T, lo T) T {
	// val 通过右移 lo 位，将要提取的位移到最低位
	// 通过左移 (hi - lo + 1) 位，创建一个掩码，该掩码的最高位是1，其余位都是0
	// 最后，将右移后的 val 和掩码进行按位与操作，提取出指定范围的位，并返回结果
	return (val >> lo) & ((1 << (hi - lo + 1)) - 1)
}

/**
 * @description: 该函数的目的是将 val 进行符号扩展，并返回扩展后的结果
 * @param {uint64} val  表示要进行符号扩展的整数值
 * @param {int} size 表示要扩展到的位数
 * @return {*}
 */
func SignExtend(val uint64, size int) uint64 {
	// val 通过左移 (63 - size) 位，将要扩展的位移到最高位
	// 然后，通过右移 (63 - size) 位，将最高位的符号位扩展到所有位上，实现符号扩展
	// 将扩展后的结果转换为 uint64 类型，并返回结果
	return uint64(int64(val << (63 - size)) >> (63 - size))
}