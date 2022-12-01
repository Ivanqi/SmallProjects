// 随机访问存储器 RAM
//  1. 它是易失性存储器，通常都是在掉电之后就会丢失数据
module dmem (
    input[11:0] addr,
    input we,   // 写使能信号（we）为“1”时，才能往 RAM 里写数据，否则只能读取数据
    input[31:0] din,
    input clk,  // 时钟信号 clk

    output reg[31:0] dout
);
    reg[31:0] dmem_reg[0:4095];

    always @(posedge clk) begin
        if (we) begin
            dmem_reg[addr] <= din;
        end
        dout <= dmem_reg[addr];
    end   
endmodule

// ROM 功能的代码
module imem (
    input[11:0] addr1,
    output[31:0] imem_o1,
    input[11:0] addr2,
    output[31:0] imem_o2
);
    // 使用了寄存器（reg）临时定义了一个指令存储器 imem
    // 并在仿真的顶层（tb_top.v）使用了 $readmemh（）函数
    // 把编译好的二进制指令读入到 imem 中，以便 CPU 内部读取并执行这些指令
    reg[31:0] imem_reg[0:4096];

    assign imem_o1 = imem_reg[addr1];
    assign imem_o2 = imem_reg[addr2];
    
endmodule