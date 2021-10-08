package consistenthashing

import (
	"errors"
	"hash/crc32"
	"hash/fnv"
	"sort"
	"strconv"
	"sync"
)

type uints []uint32			// 32位无符号数组

// Len返回uints数组的长度
func (x uints) Len() int {
	return len(x)
}

// 如果元素i小于元素j，则Less返回true。
func (x uints) Less(i, j int) bool {
	return x[i] < x[j]
}

// 交换元素i和j
func (x uints) Swap(i, j int) {
	x[i], x[j] = x[j], x[i]
}

// ErrEmptyCircle是在哈希中未添加任何内容的情况下尝试获取元素时返回的错误
var ErrEmptyCircle = errors.New("empty circle")

// Consistent保存有关一致散列循环成员的信息
type Consistent struct {
	Circle           map[uint32]string	// hash环，key为hash值，值存放的是节点信息
	Members          map[string]bool	// 存储元素成员的成员列表
	SortedHashes     uints				// 已经排序的节点hash切片
	NumberOfReplicas int				// 虚拟节点个数，用来增加hash的平衡性
	Count            int64				// 统计数量
	Scratch          [64]byte
	UseFnv           bool
	sync.RWMutex						// 读写锁
}

/*
	新建创建一个新的一致对象，默认设置为每个条目20个副本
	要更改副本的数量，请在添加条目之前设置NumberOfReplicas
*/
func New() *Consistent {
	c := new(Consistent)
	c.NumberOfReplicas = 20
	c.Circle = make(map[uint32]string)
	c.Members = make(map[string]bool)
	return c
}

func (c *Consistent) EltKey(elt string, idx int) string {
	return c.eltKey(elt, idx);
}

// eltKey为具有索引的元素生成字符串键
func (c *Consistent) eltKey(elt string, idx int) string {
	// return elt + "|" + strconv.Itoa(idx)
	// strconv.Itoa 数字转换成对应的字符串类型的数字
	return strconv.Itoa(idx) + elt
}

// Add在一致散列中插入字符串元素
func (c *Consistent) Add(elt string) {
	c.Lock()
	// defer后面的语句不会马上调用, 而是延迟到函数结束时调用
	defer c.Unlock()
	c.add(elt)
}

// 调用前需要c.Lock()
func (c *Consistent) add(elt string) {
	for i := 0; i < c.NumberOfReplicas; i++ {
		c.Circle[c.hashKey(c.eltKey(elt, i))] = elt
	}
	c.Members[elt] = true
	c.updateSortedHashes()
	c.Count++
}

// 从hash中删除元素
func (c *Consistent) Remove(elt string) {
	c.Lock()
	defer c.Unlock()
	c.remove(elt)
}

// 调用前先执行c.Lock()
func (c *Consistent) remove(elt string) {
	for i := 0; i < c.NumberOfReplicas; i++ {
		delete(c.Circle, c.hashKey(c.eltKey(elt, i)))
	}

	delete(c.Members, elt)
	c.updateSortedHashes()
	c.Count--
}

// 更新排序，方便查找
func (c *Consistent) updateSortedHashes() {
	hashes := c.SortedHashes[:0]
	// 判断切片容量是否过大，如果过大则重置，重新分配（1/4）
	if cap(c.SortedHashes) / (c.NumberOfReplicas * 4) > len(c.Circle) {
		hashes = nil
	}

	// 添加hash
	for k := range c.Circle {
		hashes = append(hashes, k)
	}
	
	// 排序
	sort.Sort(hashes)
	c.SortedHashes = hashes
}

// Set设置散列中的所有元素。如果存在不可用的现有元素
// 如果出现在ELT中，它们将被删除
func (c *Consistent) Set(elts []string) {
	c.Lock()
	defer c.Unlock()

	for k := range c.Members {
		found := false
		for _, v := range elts {
			if k == v {
				found = true
				break
			}
		}

		if !found {
			c.remove(k)
		}
	}

	for _, v := range elts {
		_, exists := c.Members[v]
		if exists {
			continue
		}
		c.add(v)
	}
}

func (c *Consistent) MembersList() []string {
	c.RLock()
	defer c.RUnlock()

	var m []string

	for k := range c.Members {
		m = append(m, k)
	}

	return m
}

// Get返回一个元素，该元素靠近名称在圆中散列的位置. 根据数据表示，获取对应的服务器节点信息
func (c *Consistent) Get(name string) (string, error) {
	c.RLock()
	defer c.RUnlock()

	if len(c.Circle) == 0 {
		return "", ErrEmptyCircle
	}

	key := c.hashKey(name)
	i := c.search(key)

	return c.Circle[c.SortedHashes[i]], nil
}

// 顺时针查找最近的节点
func (c *Consistent) search(key uint32) (i int) {
	f := func(x int) bool {
		return c.SortedHashes[x] > key
	}

	i = sort.Search(len(c.SortedHashes), f)

	if i >= len(c.SortedHashes) {
		i = 0
	}

	return 
}

// GetTwo返回与圆中输入的名称最接近的两个不同元素
func (c *Consistent) GetTwo(name string) (string, string, error) {
	c.RLock()
	defer c.RUnlock()

	if len(c.Circle) == 0 {
		return "", "", ErrEmptyCircle
	}

	key := c.hashKey(name)
	i := c.search(key)
	a := c.Circle[c.SortedHashes[i]]

	if c.Count == 1 {
		return a, "", nil
	}

	start := i
	var b string

	for i = start + 1; i != start; i++ {
		if i >= len(c.SortedHashes) {
			i = 0
		}

		b = c.Circle[c.SortedHashes[i]]

		if b != a {
			break
		}
	}

	return a, b, nil
}

func sliceContainsMember(set []string, member string) bool {
	for _, m := range set {
		if m == member {
			return true
		}
	}

	return false
}

// GetN返回与圆中输入的名称最近的N个不同元素
func (c *Consistent) GetN(name string, n int) ([]string, error) {
	c.RLock()
	defer c.RUnlock()

	if len(c.Circle) == 0 {
		return nil, ErrEmptyCircle
	}

	if c.Count < int64(n) {
		n = int(c.Count)
	}

	var (
		key   = c.hashKey(name)
		i     = c.search(key)
		start = i
		res   = make([]string, 0, n)
		elem  = c.Circle[c.SortedHashes[i]]
	)

	res = append(res, elem)

	if len(res) == n {
		return res, nil
	}

	for i = start + 1; i != start; i++ {
		if i >= len(c.SortedHashes) {
			i = 0
		}

		elem = c.Circle[c.SortedHashes[i]]

		if !sliceContainsMember(res, elem) {
			res = append(res, elem)
		}

		if len(res) == n {
			break
		}
	}

	return res, nil
}

func (c *Consistent) HashKey(key string) uint32 {
	return c.hashKey(key)
}

func (c *Consistent) hashKey(key string) uint32 {
	if c.UseFnv {
		return c.hashKeyFnv(key)
	}

	return c.hashKeyCRC32(key)
}


func (c *Consistent) hashKeyCRC32(key string) uint32 {
	if len(key) < 64 {
		var scratch [64]byte
		// copy() 可以将一个数组切片复制到另一个数组切片中，如果加入的两个数组切片不一样大，就会按照其中较小的那个数组切片的元素个数进行复制
		// copy( destSlice, srcSlice []T) int
		copy(scratch[:], key)
		return crc32.ChecksumIEEE(scratch[:len(key)])
	}
	return crc32.ChecksumIEEE([]byte(key))

}

/*
	先初始化 hash，然后循环 乘以素数 prime32，再与每位 byte 进行异或运算
	https://segmentfault.com/a/1190000016933879
*/
func (c *Consistent) hashKeyFnv(key string) uint32 {
	// New32a 返回一个新的32位 FNV-1a hash.Hash 。它的 Sum 方法将以 big-endian 字节顺序排列值
	h := fnv.New32a()
	h.Write([]byte(key))
	return h.Sum32()
}