package linker

type Chunker interface {
	GetShdr() *Shdr
	CopyBuf(ctx *Context)
}

// 基类
type Chunk struct {
	Name string
	Shdr Shdr
}

func NewChunk() Chunk {
	// 默认: 1字节对齐
	return Chunk{Shdr: Shdr{AddrAlign: 1}}
}

func (c *Chunk) GetShdr() *Shdr {
	return &c.Shdr
}

func (c *Chunk) CopyBuf(ctx *Context) {}
