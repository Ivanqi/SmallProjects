package linker

type Chunker interface {
	GetName() string
	GetShdr() *Shdr
	UpdateShdr(ctx *Context) // 更新chunk内 section header
	GetShndx() int64         // 返回chunk 所对应的可执行文件所对应section header对应的index
	CopyBuf(ctx *Context)
}

// 基类
type Chunk struct {
	Name  string
	Shdr  Shdr
	Shndx int64
}

func NewChunk() Chunk {
	// 默认: 1字节对齐
	return Chunk{Shdr: Shdr{AddrAlign: 1}}
}

func (c *Chunk) GetName() string {
	return c.Name
}

func (c *Chunk) GetShdr() *Shdr {
	return &c.Shdr
}

func (c *Chunk) UpdateShdr(ctx *Context) {}

func (c *Chunk) GetShndx() int64 {
	return c.Shndx
}

func (c *Chunk) CopyBuf(ctx *Context) {}
