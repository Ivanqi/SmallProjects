// 写回控制模块
//  1. 选择存储器读取回来的数据作为写回的结果
//  2. 还是选择流水线执行运算之后产生的数据作为写回结果
module mem_wb_ctrl (
    input   clk,
    input   reset,
    input   in_wb_ctrl_toReg,
    input   in_wb_ctrl_regWrite,

    output  data_wb_ctrl_toReg,
    output  data_wb_ctrl_regWrite
);

    // 写回寄存器的数据选择信号 wb_ctrl_toReg
    //  1. 当这个信号为“1”时，选择从存储器读取的数值作为写回数据，否则把流水线的运算结果作为写回数据
    reg reg_wb_ctrl_toReg;

    // 写控制信号 wb_ctrl_regWrite，当这个信号为“1”时，开始往目标寄存器写回指令执行的结果
    reg reg_wb_ctrl_regWrite;

    assign data_wb_ctrl_toReg = reg_wb_ctrl_toReg;
    assign data_wb_ctrl_regWrite = reg_wb_ctrl_regWrite;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            reg_wb_ctrl_toReg <= 1'h0;
        end else begin
            reg_wb_ctrl_toReg <= in_wb_ctrl_toReg;
        end
    end

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            reg_wb_ctrl_regWrite <= 1'h0;
        end else begin
            reg_wb_ctrl_regWrite <= in_wb_ctrl_regWrite; 
        end
    end
    
endmodule