package linker

type ContextArgs struct {
	Output       string      // 输出值
	Emulation    MachineType // 机器类型
	LibraryPaths []string    // 第三方扩展路径数组
}

type Context struct {
	Args ContextArgs
	Buf  []byte

	Ehdr *OutputEhdr
	Shdr *OutputShdr
	// Phdr *OutputPhdr
	// Got  *GotSection

	// TpAddr uint64

	OutputSections []*OutputSection

	Chunks []Chunker

	Objs           []*ObjectFile
	SymbolMap      map[string]*Symbol // SymbolMap key为symbol的名字，val 为 Symbol struct
	MergedSections []*MergedSection   // 存放这所有merged section，最终写到文件中
	InternalObj    *ObjectFile
	InternalEsyms  []Sym
}

func NewContext() *Context {
	return &Context{
		Args: ContextArgs{
			Output:    "a.out",
			Emulation: MachineTypeNone,
		},
		SymbolMap: make(map[string]*Symbol),
	}
}
