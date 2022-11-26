// ALU 模块
//  1. 执行控制模块根据三类指令产生的 ALU 操作信号 aluOp
//  2. 在 ALU 模块就能以此为依据，执行相应的运算了
module alu(
    input[31:0] alu_data1_i,
    input[31:0] alu_data2_i,
    input[3:0]  alu_op_i,
    
    output[31:0]    alu_result_o
);

    reg[31:0] result;

    // sum 信号其实就是两个源操作数的和，不过当运算码 aluOp 的第 3 位和第 1 位为“1”时做的是相减运算
    // 这是为减法指令或者后面的比较大小而准备的运算
    wire[31:0]  sum = alu_data1_i + ((alu_op_i[3] | alu_op_i[1]) ? -alu_data2_i : alu_data2_i);

    // neq 信号表示的是比较两个操作数是否相等，这就是根据前面的两个操作相减的结果判断
    // 如果它们的差不为“0”，也就是 sum 信号按位与之后不为“0”，则表示两个操作数不相等
    wire        neq = | sum;

    // cmp 信号表示两个操作数的大小比较，如果它们的最高位（也就是符号位）相等
    // 则根据两个操作数相减的差值的符号位（也是数值的最高位）判断
    // 如果是正数，表示源操作数 1 大于源操作数 2，否则表示源操作数 1 小于源操作数 2

    // 如果它们的最高位不相等，则根据 ALU 运算控制码 aluOp 的最低位判断
    // 如果 aluOp 最低位为“1”，表示是无符号数比较，直接取操作数 2 的最高位作为比较结果
    // 如果 aluOp 最低位为“0”，表示是有符号数比较，直接取操作数 1 的最高位作为比较结果
    wire        cmp = (alu_data1_i[31] == alu_data2_i[31]) ? sum[31] 
                    : alu_op_i[0] ? alu_data2_i[31] : alu_data1_i[31];
    
    // 移位操作相关的代码，其中的 shamt 信号是取自源操作数 2 的低五位，表示源操作数 1 需要移多少位（25=32）
    // shin 信号是取出要移位的数值，根据 aluOp 判断是左移还是右移，如果是右移就直接等于源操作数 1
    // 如果是左移就先对源操作数的各位数做镜像处理
    wire[4:0]   shamt = alu_data2_i[4:0];
    wire[31:0]  shin  = alu_op_i[2] ? alu_data1_i: reverse(alu_data1_i);

    // shift 信号是根据 aluOp 判断是算术右移还是逻辑右移，如果是算术右移，则在最高位补一个符号位
    // shiftt 信号是右移之后的结果，这里用到了$signed() 函数对移位前的数据 shift 进行了修饰
    // $signed() 的作用是决定如何对操作数扩位这个问题
    wire[32:0]  shift = {alu_op_i[3] & shin[31], shin};

    // 在右移操作前，$signed() 函数先把操作数的符号位，扩位成跟结果相同的位宽
    // 然后再进行移位操作，而 shiftr 就是右移后的结果
    wire[32:0]  shiftt = ($signed(shift) >>> shamt);
    wire[31:0]  shiftr  = shiftt[31:0];
    // 左移的结果 shiftl，是由右移后的结果进行位置取反得到的
    wire[31:0]  shiftl  = reverse(shiftr);

    always @(*) begin
        case(alu_op_i)
            `ALU_OP_ADD:    result <= sum;  // 加法运算add
            `ALU_OP_SUB:    result <= sum;  // 减法运算sub
            `ALU_OP_SLL:    result <= shiftl;   // 左移位shiftl
            `ALU_OP_SLT:    result <= cmp;      // 小于
            `ALU_OP_SLTU:   result <= cmp;      // 小于
            `ALU_OP_XOR:    result <= (alu_data1_i ^ alu_data2_i);  // 异或
            `ALU_OP_SRL:    result <= shiftr;   // 右移位shiftr
            `ALU_OP_SRA:    result <= shiftr;   // 右移位shiftr
            `ALU_OP_OR:     result <= (alu_data1_i | alu_data2_i);  // 逻辑或|
            `ALU_OP_AND:    result <= (alu_data1_i & alu_data2_i);  // 逻辑与&

            `ALU_OP_EQ:     result <= {31'b0, ~neq};    // 相等
            `ALU_OP_NEQ:    result <= {31'b0, neq};     // 不相等 
            `ALU_OP_GE:     result <= {31'b0, ~cmp};    // 大于
            `ALU_OP_GEU:    result <= {31'b0, ~cmp};    // 大于

            default:        begin
                            result <= 32'b0;
                            //$display("*** alu error ! ***%x", alu_op_i); 
            end
        endcase
    end

    function [31:0] reverse;
        input[31:0] in;
        integer i;
        for (i = 0; i < 32; i = i+1) begin
            reverse[i] = in[31-i];
        end
    endfunction

    assign alu_result_o = result;

endmodule