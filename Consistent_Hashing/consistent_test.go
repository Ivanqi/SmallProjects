package consistenthashing

import (
	"sort"
	"testing"
)

func checkNum(num, expected int, t *testing.T) {
	if num != expected {
		t.Errorf("go %d, expected %d", num, expected)
	}
}

func TestNew(t *testing.T) {
	x := New()
	if x == nil {
		t.Errorf("expected obj")
	}
	checkNum(x.NumberOfReplicas, 20, t)
}
func TestAdd(t *testing.T) {
	x := New()
	x.Add("abcdefg")
	checkNum(len(x.circle), 20, t)
	checkNum(len(x.sortedHashes), 20, t)
	if sort.IsSorted(x.sortedHashes) == false {
		t.Errorf("expected sorted hashes to be sorted")
	}
	x.Add("qwer")
	checkNum(len(x.circle), 40, t)
	checkNum(len(x.sortedHashes), 40, t)
	if sort.IsSorted(x.sortedHashes) == false {
		t.Errorf("expected sorted hashes to be sorted")
	}
}
