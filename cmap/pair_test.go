package cmap

import "testing"

// keyElement 用于存储键 - 元素对
type keyElement struct {
	key     string
	element interface{}
}

// randElement 会生成并返回一个伪随机元素值
func randElement() interface{} {
	if i := rand.Int31(); i%3 != 0 {
		return i
	}
	buf := new(bytes.Buffer)
	binary.Write(buf, binary.LittleEndian, rand.Int31)
	// hex. EncodeToString 返回十六进制编码
	return hex.EncodeToString(buf.Bytes())
}

// randString 会生成并返回一个伪随机字符串
func randString() string {
	buf := new(bytes.Buffer)
	binary.Write(buf, binary.LittleEndian, rand.Int31())
	return hex.EncodeToString(buf.Bytes())
}

// genTestingKeyElementSlice 用于生成测试用的键-元素对的切片
func genTestingKeyElementSlice(number int) []*keyElement {
	testCases := make([]*keyElement, number)
	for i := 0; i < number; i++ {
		testCases[i] = &keyElement{randString(), randElement()}
	}
	return testCases
}

func TestPairNew(t *testing.T) {
	testCases := genTestingKeyElementSlice(100)
}
