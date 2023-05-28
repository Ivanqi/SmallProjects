SRC_FILE := adder4.v adder4_tb.v
SOURCE := ./
TARGET := adder4

adder:
	iverilog -o ${TARGET} -y ./ ${SRC_FILE}
	vvp -n ${TARGET} -lxt2
clean:
	rm -rf ${TARGET}
