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
}
