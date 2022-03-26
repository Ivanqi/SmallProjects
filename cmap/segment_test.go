package cmap

import (
	"fmt"
	"testing"
)

func TestSegmentNew(t *testing.T) {
	s := newSegment(-1, nil)
	if s == nil {
		t.Fatal("Couldn't new segment")
	}
}

func TestSegmentPut(t *testing.T) {
	number := 30
	testCases := genTestingPairs(number)
	s := newSegment(-1, nil)
	var count uint64

	for _, p := range testCases {
		ok, err := s.Put(p)

		if err != nil {
			t.Fatalf("An error occurs when putting a pair to the segment: %s (pair: %#v)", err, p)
		}

		if !ok {
			t.Fatalf("Couldn't put pair to the segment! (pair: %#v)", p)
		}

		actualPair := s.Get(p.Key())
		if actualPair == nil {
			t.Fatalf("Inconsistent pair: expected: %#v, actual: %#v", p.Element(), nil)
		}

		ok, err = s.Put(p)
		if err != nil {
			t.Fatalf("An error occurs when putting a repeated pair to the segment: %s (pair: %#v)", err, p)
		}

		if ok {
			t.Fatalf("Couldn't put repeated pair to the segment! (pair: %#v)", p)
		}

		count++
		if s.Size() != count {
			t.Fatalf("Inconsistent size: expected: %d, actual: %d", count, s.Size())
		}
	}

	if s.Size() != uint64(number) {
		t.Fatalf("Inconsistent size: expected: %d, actual: %d", number, s.Size())
	}
}

func TestSegmentPutInParallel(t *testing.T) {
	number := 30
	testCases := genNoRepetitiveTestingPairs(number)
	s := newSegment(-1, nil)
	testingFunc := func(p Pair, t *testing.T) func(t *testing.T) {
		return func(t *testing.T) {
			t.Parallel()
			ok, err := s.Put(p)
			if err != nil {
				t.Fatalf("An error occurs when putting a pair to the segment: %s (pair: %#v)", err, p)
			}

			if !ok {
				t.Fatalf("Couldn't put a pair to the segment! (pair: %#v)", p)
			}

			actualPair := s.Get(p.Key())
			if actualPair == nil {
				t.Fatalf("Inconsistent pair: expected: %#v, actual: %#v", p.Element(), nil)
			}

			ok, err = s.Put(p)
			if err != nil {
				t.Fatalf("An error occurs when putting a repeated pair to the segment: %s (pair: %#v)", err, p)
			}
		}
	}

	t.Run("Put in parallel", func(t *testing.T) {
		for _, p := range testCases {
			t.Run(fmt.Sprintf("Key=%s", p.Key()), testingFunc(p, t))
		}
	})

	if s.Size() != uint64(number) {
		t.Fatalf("Inconsistent size: expected: %d, actual: %d", number, s.Size())
	}
}

func TestSegmentGetInParallel(t *testing.T) {
	number := 30
	testCases := genNoRepetitiveTestingPairs(number)
	s := newSegment(-1, nil)

	for _, p := range testCases {
		s.Put(p)
	}

	testingFunc := func(p Pair, t *testing.T) func(t *testing.T) {
		return func(t *testing.T) {
			t.Parallel()
			actualPair := s.Get(p.Key())
			if actualPair == nil {
				t.Fatalf("Not found pair in segment! (key: %s)", p.Key())
			}

			if actualPair.Key() != p.Key() {
				t.Fatalf("Inconsistent key: expected: %s, actual: %s", p.Key(), actualPair.Key())
			}

			if actualPair.Hash() != p.Hash() {
				t.Fatalf("Inconsistent hash: expected: %d, actual: %d", p.Hash(), actualPair.Hash())
			}

			if actualPair.Element() != p.Element() {
				t.Fatalf("Inconsistent element: expected: %#v, actual: %#v", p.Element(), actualPair.Element())
			}
		}
	}

	t.Run("Get in parallel", func(t *testing.T) {
		t.Run("Put in parallel", func(t *testing.T) {
			for _, p := range testCases {
				s.Put(p)
			}
		})

		for _, p := range testCases {
			t.Run(fmt.Sprintf("Get: Key=%s", p.Key()), testingFunc(p, t))
		}
	})

	if s.Size() != uint64(number) {
		t.Fatalf("Inconsistent size: expected: %d, actual: %d", number, s.Size())
	}
}

func TestSegmentDelete(T *testing.T) {
	number := 30
	testCases := genTestingPairs(number)
	s := newSegment(-1, nil)
	for _, p := range testCases {
		s.Put(p)
	}

	count := uint64(number)
	for _, p := range testCases {
		done := s.Delete(p.Key())
		if !done {
			t.Fatalf("Couldn't delete a pair from segment again! (pair: %#v)", p)
		}

		if count > 0 {

		}
	}
}
