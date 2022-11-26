// 执行控制模块
module alu_ctrl (
    input[2:0]  funct3,
    input[6:0]  funct7,
    input[1:0]  aluCtrlOp,
    input       itype,      // I 型指令类型的判断信号 itype
    output reg[3:0] aluOp   // 一个4位的ALU操作信号
);
    always @(*) begin
        case(aluCtrlOp)
            2'b00:  aluOp <= `ALU_OP_ADD;   // 读写寄存器
            2'b01:  begin                   // 普通算术运算 
                // 根据输入的 funct7 和 funct3 字段决定执行哪些算术运算
                // 比如加减运算、移位操作等
                // 如果是 itype 信号等于“1”，操作码直接由 funct3 和高位补“0”组成
                // 如果不是 I 型指令，ALU 操作码则要由 funct3 和 funct7 的第五位组成
                if (itype & funct3[1:0] != 2'b01)
                    aluOp <= {1'b0, funct3};
                else
                    aluOp <= {funct7[5], funct3};    // normal ALUI/ALUR
            end
            2'b10:  begin                   // 条件分支解析
                // $display("~~~aluCtrl bxx~~~%d", funct3);
                case (funct3)               // bxx
                    `BEQ_FUNCT3:    aluOp   <=  `ALU_OP_EQ;
                    `BNE_FUNCT3:    aluOp   <=  `ALU_OP_NEQ;
                    `BLT_FUNCT3:    aluOp   <=  `ALU_OP_SLT;
                    `BGE_FUNCT3:    aluOp   <=  `ALU_OP_GE;
                    `BLTU_FUNCT3:   aluOp   <=  `ALU_OP_SLTU;
                    `BGEU_FUNCT3:   aluOp   <=  `ALU_OP_GEU;
                    default:    aluOp   <=  `ALU_OP_XXX;
                endcase
                end
            default:    aluOp   <= `ALU_OP_XXX; // 其他操作
        endcase
    end
endmodule