// 译码模块
// 1. 译码模块就是拆解从取指模块传过来的每一条指令
// 2. 译码时，需要识别出指令的操作码，并根据对应的指令格式提取出指令中包含的信息
module decode (
    input[31:0]     instr,      // 指令源码

    output[4:0]     rs1_addr,   // 源寄存器rs1索引
    output[4:0]     rs2_addr,   // 源寄存器rs2索引
    output[4:0]     rd_addr,    // 目标寄存器rd索引
    output[2:0]     funct3,     // 功能码funct3
    output[6:0]     funct7,     // 功能码funct7
    output          branch,     // 产生分支信息branch
    output[1:0]     jump,       // 跳转信号jump 
    output          mem_read,   // 读存储器信号 mem_read
    output          mem_write,
    output          reg_write,
    output          to_reg,
    output[1:0]     result_sel,
    output          alu_src,
    output          pc_add,
    output[6:0]     types,
    output[1:0]     alu_ctrlop,
    output          valid_inst,
    output[31:0]    imm    
)

localparam DEC_INVALID = 21'b0;

reg[20:0]   dec_array;

// 负责完成指令的源寄存器、目标寄存器、3 位操作码和 7 位操作码的译码
//---------- decode rs1、rs2.源寄存器 -----------------
assign rs1_addr = instr[19:15];
assign rs2_addr = instr[24:20];

//---------- decode rd.目标寄存器 -----------------------
assign rd_addr = instr[11:7];

//---------- decode funct3、funct7 -----------
assign funct7 = instr[31:25];   // 7 位操作码
assign funct3 = instr[14:12];   // 3 位的操作码

// 负责完成指令格式类型的识别
// ----------------------------- decode signals ---------------------------------

//                        20     19-18  17       16        15        14     13-12      11      10     9--------3  2---1      0
//                        branch jump   memRead  memWrite  regWrite  toReg  resultSel  aluSrc  pcAdd     RISBUJZ  aluctrlop  validInst
localparam DEC_LUI     = {1'b0,  2'b00, 1'b0,    1'b0,     1'b1,     1'b0,  2'b01,     1'b0,   1'b0,  7'b0000100, 2'b00,     1'b1};
localparam DEC_AUIPC   = {1'b0,  2'b00, 1'b0,    1'b0,     1'b1,     1'b0,  2'b00,     1'b1,   1'b1,  7'b0000100, 2'b00,     1'b1};
localparam DEC_JAL     = {1'b0,  2'b00, 1'b0,    1'b0,     1'b1,     1'b0,  2'b10,     1'b0,   1'b0,  7'b0000010, 2'b00,     1'b1};
localparam DEC_JALR    = {1'b0,  2'b11, 1'b0,    1'b0,     1'b1,     1'b0,  2'b10,     1'b1,   1'b0,  7'b0100000, 2'b00,     1'b1};
localparam DEC_BRANCH  = {1'b1,  2'b00, 1'b0,    1'b0,     1'b0,     1'b0,  2'b00,     1'b0,   1'b0,  7'b0001000, 2'b10,     1'b1};
localparam DEC_LOAD    = {1'b0,  2'b00, 1'b1,    1'b0,     1'b1,     1'b1,  2'b00,     1'b1,   1'b0,  7'b0100000, 2'b00,     1'b1};
localparam DEC_STORE   = {1'b0,  2'b00, 1'b0,    1'b1,     1'b0,     1'b0,  2'b00,     1'b1,   1'b0,  7'b0010000, 2'b00,     1'b1};
localparam DEC_ALUI    = {1'b0,  2'b00, 1'b0,    1'b0,     1'b1,     1'b0,  2'b00,     1'b1,   1'b0,  7'b0100000, 2'b01,     1'b1};
localparam DEC_ALUR    = {1'b0,  2'b00, 1'b0,    1'b0,     1'b1,     1'b0,  2'b00,     1'b0,   1'b0,  7'b1000000, 2'b01,     1'b1};

assign  {branch, jump, mem_read, mem_write, reg_write, to_reg, result_sel, alu_src, pc_add, types, alu_ctrlop, valid_inst} = dec_array;

// OPCODE_LUI:  U型指令、用于长立即数操作
// OPCODE_AUIPC: U型指令、用于长立即数操作
// OPCODE_JAL: J型指令、用于无条件跳转操作
// OPCODE_JALR: J型指令、用于无条件跳转操作
// OPCODE_BRANCH: B型指令、用于条件跳转操作
// OPCODE_LOAD: I型指令、短立即数及内存访问操作
// OPCODE_STORE: S型指令、用于内存store操作
// OPCODE_ALUI: I型指令、短立即数及内存访问操作
// OPCODE_ALUR: R型指令、用于寄存器- 寄存器之间的操作
always @(*) begin
    //$write("%x", instr);
    case (instr[6:0])
        `OPCODE_LUI    :   dec_array <= DEC_LUI;
        `OPCODE_AUIPC  :   dec_array <= DEC_AUIPC; 
        `OPCODE_JAL    :   dec_array <= DEC_JAL; 
        `OPCODE_JALR   :   dec_array <= DEC_JALR;   
        `OPCODE_BRANCH :   dec_array <= DEC_BRANCH; 
        `OPCODE_LOAD   :   dec_array <= DEC_LOAD;   
        `OPCODE_STORE  :   dec_array <= DEC_STORE;  
        `OPCODE_ALUI   :   dec_array <= DEC_ALUI;  
        `OPCODE_ALUR   :   dec_array <= DEC_ALUR;  
        default        :   begin
            dec_array <= DEC_INVALID;
            //  $display("~~~decode error~~~%x", instr);
        end 
    endcase
end

// 负责完成立即数译码
// -------------------- IMM -------------------------
wire [31:0] Iimm = {{21{instr[31]}}, instr[30:20]};
wire [31:0] Simm = {{21{instr[31]}}, instr[30:25], instr[11:7]};
wire [31:0] Bimm = {{20{instr[31]}}, instr[7], instr[30:25], instr[11:8], 1'b0};
wire [31:0] Uimm = {instr[31:12], 12'b0};
wire [31:0] Jimm = {{12{instr[31]}}, instr[19:12], instr[20], instr[30:21], 1'b0};  

assign imm = {32{types[5]}} & Iimm
           | {32{types[4]}} & Simm
           | {32{types[3]}} & Bimm
           | {32{types[2]}} & Uimm
           | {32{types[1]}} & Jimm;

endmodule