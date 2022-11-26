// 预读取模块
// 1. 条件跳转指令执行时是否发生跳转，要根据相关的数据来判断，这就需要指令执行之后才能知道是否需要跳转
// 2. 指令预读取模块. 不管指令是否跳转，都提前把跳转之后的下一条指令从存储器中读取出来，以备流水线的下一阶段使用
module pre_if (
    input[31:0] instr,
    input[31:0] pc,
    
    output[31:0] pre_pc
);

    wire is_bxx = (instr[6:0] == `OPCODE_BRANCH);   // 条件跳转指令的操作码
    // 无条件跳转指令就是不需要判断其他的任何条件，直接跳转
    wire is_jal = (instr[6:0] == `OPCODE_JAL);      // 无条件跳转指令的操作码
    
    // B型指令的立即数拼接
    // bimm如果是负数就转跳，如果是正数就不转跳。相当于只往回跳不往后跳，是一种分支预测
    wire[31:0] bimm = {{20{instr[31]}}, instr[7], instr[30:25], instr[11:8], 1'b0};
    // J型指令的立即数拼接
    wire[31:0] jimm = {{12{instr[31]}}, instr[19:12], instr[20], instr[30:21], 1'b0};

    // 指令地址的偏移量
    wire[31:0] adder = is_jal ? jimm : (is_bxx & bimm[31]) ? bimm : 4;
    // 预读取电路会根据当前的 PC 值和指令的偏移量相加，得到预测的 PC 值，并用预测的 PC 值提前读出下一条指令
    assign pre_pc = pc + adder;
endmodule