package linker

import (
	"reflect"
	"testing"
)

func TestGetFileType(t *testing.T) {
	type args struct {
		contents []byte
	}
	tests := []struct {
		name string
		args args
		want FileType
	}{
		// TODO: Add test cases.
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if got := GetFileType(tt.args.contents); !reflect.DeepEqual(got, tt.want) {
				t.Errorf("GetFileType() = %v, want %v", got, tt.want)
			}
		})
	}
}
