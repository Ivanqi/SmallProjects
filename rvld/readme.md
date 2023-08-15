# ELF 格式
![ELF](./imgs/ELF文件格式.png)
- Program header table: 固定长度的数组，可以直接映射到数组中去。用于运行时
- Section header table: 用于链接的时候

# ELF 整体布局
![ELF 整体布局](./imgs/elf文件.png)

# readelf 工具来查看文件的section header
readelf -S out/tests/hello/a.o
```
There are 11 section headers, starting at offset 0x2c0:

Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] .text             PROGBITS         0000000000000000  00000040
       0000000000000024  0000000000000000  AX       0     0     2
  [ 2] .rela.text        RELA             0000000000000000  000001d8
       0000000000000090  0000000000000018   I       8     1     8
  [ 3] .data             PROGBITS         0000000000000000  00000064
       0000000000000000  0000000000000000  WA       0     0     1
  [ 4] .bss              NOBITS           0000000000000000  00000064
       0000000000000000  0000000000000000  WA       0     0     1
  [ 5] .rodata           PROGBITS         0000000000000000  00000068
       000000000000000d  0000000000000000   A       0     0     8
  [ 6] .comment          PROGBITS         0000000000000000  00000075
       0000000000000028  0000000000000001  MS       0     0     1
  [ 7] .note.GNU-stack   PROGBITS         0000000000000000  0000009d
       0000000000000000  0000000000000000           0     0     1
  [ 8] .symtab           SYMTAB           0000000000000000  000000a0
       0000000000000120  0000000000000018           9    10     8
  [ 9] .strtab           STRTAB           0000000000000000  000001c0
       0000000000000015  0000000000000000           0     0     1
  [10] .shstrtab         STRTAB           0000000000000000  00000268
       0000000000000052  0000000000000000           0     0     1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  p (processor specific)
```

# readelf 工具来查看文件的symbol table
```
root@4ec3c2798e0f:/code# readelf -s out/tests/hello/a.o

Symbol table '.symtab' contains 12 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    1
     3: 0000000000000000     0 SECTION LOCAL  DEFAULT    3
     4: 0000000000000000     0 SECTION LOCAL  DEFAULT    4
     5: 0000000000000000     0 SECTION LOCAL  DEFAULT    5
     6: 0000000000000000     0 SECTION LOCAL  DEFAULT    7
     7: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT    5 .LC0
     8: 0000000000000008     0 NOTYPE  LOCAL  DEFAULT    1 .L0
     9: 0000000000000000     0 SECTION LOCAL  DEFAULT    6
    10: 0000000000000000    36 FUNC    GLOBAL DEFAULT    1 main
    11: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND puts
```