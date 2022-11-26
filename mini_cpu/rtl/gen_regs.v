// 通用寄存器
module gen_regs (
    input clk,
    input reset,
    input wen,  // 写控制使能信号
    input[4:0] regRAddr1, regRAddr2, regWAddr,
    input[31:0] regWData,

    output[31:0] regRData1,
    output[31:0] regRData2
);
    integer ii;
    // 通用寄存器总共有 32 个
    reg[31:0] regs[31:0];

    // write registers
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            for(ii=0; ii < 32; ii=ii+1)
                regs[ii] <= 32'b0;
        end
        else if (wen & (|regWAddr))
            regs[regWAddr] <= regWData;
    end

    // 因为写寄存器是边沿触发的，在一个时钟周期内写入的寄存器数据，需要在下一个时钟周期才能把写入的数据读取出来
    // 为了提高读写效率，在对同一个寄存器进行读写时，如果写使能 wen 有效，就直接把写入寄存器的数据送给读数据接口
    // 这样就可以在一个时钟周期内，读出当前要写入的寄存器数据了

    // read registers
    // 读地址 regRAddr1 = 5’b0 时的寄存器.把第一个寄存器叫做 0 值寄存器
    // 因为在 RISC-V 指令架构中就规定好了，第一个通用寄存器必须编码为 0
    // 也就是把写入该寄存器的数据忽略，而在读取时永远输出为 0
    assign regRData1 = wen & (regWAddr == regRAddr1) ? regWData
            : ((regRAddr1 != 5'b0) ? regs[regRAddr1] : 32'b0);
    
    assign regRData2 = wen & (regWAddr == regRAddr2) ? regWData
            : ((regRAddr2 != 5'b0) ? regs[regRAddr2] : 32'b0);
endmodule