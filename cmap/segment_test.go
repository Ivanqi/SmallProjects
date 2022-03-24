package cmap

import (
	"testing"

	"golang.org/x/text/number"
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
