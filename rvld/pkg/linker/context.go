package linker

type ContextArgs struct {
	Output       string      // 输出值
	Emulation    MachineType // 机器类型
	LibraryPaths []string    // 第三方扩展路径数组
}

type Context struct {
	Args ContextArgs
	Buf  []byte

	// Ehdr *OutputEhdr
	// Shdr *OutputShdr
	// Phdr *OutputPhdr
	// Got  *GotSection

	// TpAddr uint64

	// OutputSections []*OutputSection

	// Chunks []Chunker

	Objs []*ObjectFile
	// SymbolMap      map[string]*Symbol
	// MergedSections []*MergedSection
}

func NewContext() *Context {
	return &Context{
		Args: ContextArgs{
			Output:    "a.out",
			Emulation: MachineTypeNone,
		},
		// SymbolMap: make(map[string]*Symbol),
	}
}
