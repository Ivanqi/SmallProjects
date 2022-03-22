package cmap

import "testing"

// genTestingPairs 用于生成测试用的键-元素对的切片
func genTestingPairs(number int) []Pair {
	testCases := make([]Pair, number)
	for i := 0; i < number; i++ {
		testCases[i], _ = newPair(randString(), randElement())
	}

	return testCases
}

func TestBucketNew(t *testing.T) {
	b := newBucket()
	if b == nil {
		t.Fatal("Couldn't new bucket!")
	}
}

func TestBucketPut(t *testing.T) {
	number := 30
	testCases := genTestingPairs(number)
	b := newBucket()
	var count uint64

	for _, p := range testCases {
		ok, err := b.Put(p, nil)
		if err != nil {
			t.Fatalf("An error occurs when putting a pair to the bucket: %s (pair: %#v)", err, p)
		}

		if !ok {
			t.Fatalf("Couldn't put pair to the bucket! (pair: %#v)", p)
		}

		actualPair := b.Get(p.Key())
		if actualPair == nil {
			t.Fatalf("Inconsistent pair: expected: %#v, actual: %#v", p.Element(), nil)
		}

		ok, err = b.Put(p, nil)
		if err != nil {
			t.Fatalf("An error occurs when putting a repeated  pair to the bucket: %s (pair : %#v)", err, p)
		}

		if ok {
			t.Fatalf("Coundn't put repeated pair to the bucket! (pair: %#v)", p)
		}

		count++

		if b.Size() != count {
			t.Fatalf("Inconsistent size: expected: %d, actual: %d", count, b.Size())
		}
	}

	if b.Size() != uint64(number) {
		t.Fatalf("Inconsistent size: expected: %d, actual: %d", number, b.Size())
	}
}
