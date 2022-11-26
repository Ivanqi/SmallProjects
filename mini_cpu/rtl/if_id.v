// 取指通路模块
// 1. 在取指阶段就是把存储器里的指令读出，并传递给后续的译码模块进行处理
// 2. 预读取模块把指令从存储器中读取之后，需要把它发送给译码模块进行翻译
//      但是，预读取模块读出的指令，并不是全部都能发送后续模块去执行
// 3. 条件分支指令，在指令完成之前就把后续的指令预读取出来了
//      如果指令执行之后发现跳转的条件不成立，这时预读取的指令就是无效的
//      需要对流水线进行冲刷（flush），把无效的指令都清除掉
module if_id (
    input           clk,
    input           reset,
    input[31:0]     in_instr,
    input[31:0]     in_pc,
    input           flush,
    input           valid,
    
    output[31:0]    out_instr,
    output[31:0]    out_pc,
    output          out_noflush
);

    reg[31:0] reg_instr;
    reg[31:0] reg_pc;
    reg[31:0] reg_pc_next;
    reg       reg_noflush;

    assign out_instr = reg_instr;
    assign out_pc = reg_pc;
    assign out_noflush = reg_noflush;

    // 指令传递
    // 1. 首先是给后面解码模块提供的指令信号 reg_instr
    // 2. 如果流水线没有发生冲突，也就是没有发出清除信号 flush，则把预读取的指令保存，否则把指令清“0”
    always @(posedge clk or posedge reset)  begin
        if (reset) begin
            reg_instr <= 32'h0;
        end else if (flush) begin
            reg_instr <= 32'h0;
        end else if (valid) begin
            reg_instr <= in_instr;
        end
    end

    // PC值传递
    // 如果指令清除信号 flush=“0”，则把当前指令对应的 PC 值保存为 reg_pc，否则就把 reg_pc 清“0”
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            reg_pc <= 32'h0;
        end else if (flush) begin
            reg_pc <= 32'h0;
        end else if (valid) begin
            reg_pc <= in_pc;
        end
    end

    // 流水线冲刷标志位
    // 1. 当需要进行流水线冲刷时，reg_noflush=“0”，否则 reg_noflush=“1”
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            reg_noflush <= 1'h0;
        end else if (flush) begin
            reg_noflush <= 1'h0;
        end else if (valid) begin
            reg_noflush <= 1'h1;
        end
    end
    
endmodule