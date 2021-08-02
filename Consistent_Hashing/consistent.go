package consistenthashing

import (
	"errors"
	"hash/fnv"
	"sort"
	"strconv"
	"sync"
)

type uints []uint32

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
	circle           map[uint32]string
	members          map[string]bool
	sortedHashes     uints
	NumberOfReplicas int
	count            int64
	scratch          [64]byte
	UseFnv           bool
	sync.RWMutex
}

/*
	新建创建一个新的一致对象，默认设置为每个条目20个副本
	要更改副本的数量，请在添加条目之前设置NumberOfReplicas
*/
func New() *Consistent {
	c := new(Consistent)
	c.NumberOfReplicas = 20
	c.circle = make(map[uint32]string)
	c.members = make(map[string]bool)
	return c
}

// eltKey为具有索引的元素生成字符串键
func (c *Consistent) eltKey(elt string, idx int) string {
	// return elt + "|" + strcov.Itoa(idx)
	return strconv.Itoa(idx) + elt
}

// Add在一致散列中插入字符串元素
func (c *Consistent) Add(elt string) {
	c.Lock()
	defer c.Unlock()
	c.add(elt)
}

// 调用前需要c.Lock()
func (c *Consistent) add(elt string) {
	for i := 0; i < c.NumberOfReplicas; i++ {
		c.circle[c.hashKey(c.eltKey(elt, i))] = elt
	}
	c.members[elt] = true
	c.updateSortedHashes()
	c.count++
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
		delete(c.circle, c.hashKey(c.eltKey(elt, i)))
	}

	delete(c.members, elt)
	c.updateSortedHashes()
	c.count--
}

func (c *Consistent) updateSortedHashes() {
	hashes := c.sortedHashes[:0]
	// 如果我们坚持太多，重新分配（1/4）
	if cap(c.sortedHashes)/(c.NumberOfReplicas*4) > len(c.circle) {
		hashes = nil
	}

	for k := range c.circle {
		hashes = append(hashes, k)
	}
	sort.Sort(hashes)
	c.sortedHashes = hashes
}

// Set设置散列中的所有元素。如果存在不可用的现有元素
// 如果出现在ELT中，它们将被删除
func (c *Consistent) Set(elts []string) {
	c.Lock()
	defer c.Unlock()

	for k := range c.members {
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
		_, exists := c.members[v]
		if exists {
			continue
		}
		c.add(v)
	}
}

func (c *Consistent) Members() []string {
	c.Lock()
	defer c.RUnlock()

	var m []string

	for k := range c.members {
		m = append(m, k)
	}

	return m
}

// Get返回一个元素，该元素靠近名称在圆中散列的位置
func (c *Consistent) Get(name string) (string, error) {
	c.RLock()
	defer c.RUnlock()

	if len(c.circle) == 0 {
		return "", ErrEmptyCircle
	}

	key := c.hashKey(name)
	i := c.search(key)

	return c.circle[c.sortedHashes[i]], nil
}

func (c *Consistent) search(key uint32) (i int) {
	f := func(x int) bool {
		return c.sortedHashes[x] > key
	}

	i = sort.Search(len(c.sortedHashes), f)

	if i >= len(c.sortedHashes) {
		i = 0
	}

	return
}

// GetTwo返回与圆中输入的名称最接近的两个不同元素
func (c *Consistent) GetTwo(name string) (string, string, error) {
	c.RLock()
	defer c.RUnlock()

	if len(c.circle) == 0 {
		return "", "", ErrEmptyCircle
	}

	key := c.hashKey(name)
	i := c.search(key)
	a := c.circle[c.sortedHashes[i]]

	if c.count == 1 {
		return a, "", nil
	}

	start := i
	var b string

	for i = start + 1; i != start; i++ {
		if i >= len(c.sortedHashes) {
			i = 0
		}

		b = c.circle[c.sortedHashes[i]]

		if b != a {
			break
		}
	}

	return a, b, nil
}

// GetN返回与圆中输入的名称最近的N个不同元素
func (c *Consistent) GetN(name string, n int) ([]string, error) {
	c.RLock()
	defer c.RUnlock()

	if len(c.circle) == 0 {
		return nil, ErrEmptyCircle
	}

	if c.count < int64(n) {
		n = int(c.count)
	}

	var (
		key   = c.hashKey(name)
		i     = c.search(key)
		start = i
		res   = make([]string, 0, n)
		elem  = c.circle[c.sortedHashes[i]]
	)

	res = append(res, elem)

	if len(res) == n {
		return res, nil
	}

	for i = start + 1; i != start; i++ {
		if i >= len(c.sortedHashes) {
			i = 0
		}

		elem = c.circle[c.sortedHashes[i]]

		if !sliceContainsMember(res, elem) {
			res = append(res, elem)
		}

		if len(res) == n {
			break
		}
	}

	return res, nil
}

func (c *Consistent) hashKey(key string) uint32 {
	if c.UseFnv {
		return c.hashKeyFnv(key)
	}

	return c.hashKeyCRC32(key)
}

func (c *Consistent) hashKeyCRC32(key string) uint32 {
	h := fnv.New32a()
	h.Write([]byte(key))
	return h.Sum32()
}

func (c *Consistent) hashKeyFnv(key string) uint32 {
	h := fnv.New32a()
	h.Write([]byte(key))
	return h.Sum32()
}

func (c *Consistent) sliceContainsMember(set []string, member string) bool {
	for _, m := range set {
		if m == member {
			return true
		}
	}

	return false
}
