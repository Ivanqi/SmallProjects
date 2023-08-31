package linker

// 基类
type Chunk struct {
	Name string
	Shdr Shdr
}

func NewChunk() Chunk {
	// 默认: 1字节对齐
	return Chunk{Shdr: Shdr{AddrAlign: 1}}
}
