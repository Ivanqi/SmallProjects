// 设备地址空间
// 0x0000_0000 - .+     ROM  (byte to word)
// 0x1000_0000 - .+     RAM  (byte to word)
// 0x2000_0000 - .+     uart (byte to word)
// 0x3000_0000 - .+     other (byte to word)
// 0xc000_0000 - .+     Device io (byte to word)

// 系统总线
//  1. 总线就是 CPU 内核跟其他外设部件的“联络员”
//  2. 举几个例子，总线可以从 ROM 中读取指令，再交给 CPU 去执行
//  3. CPU 运行程序时的变量，也会交由总线保存到 RAM 中
//  4. 用来实现芯片与外部通信的 UART 模块，也需要通过总线跟 CPU 进行信息交换
module sys_bus (
    // cpu -> imem
    input[31:0] cpu_imem_addr,
    output[31:0] cpu_imem_data,
    output[31:0] imem_addr,             // cpu -> imem
    input[31:0] imem_data,              // imem -> cpu

    // cpu -> bus
    input[31:0] cpu_dmem_addr,          // device addr
    input[31:0] cpu_dmem_data_in,       // cpu -> device
    input       cpu_dmem_wen,           // cpu -> device
    output reg[31:0] cpu_dmem_data_out, // device -> cpu

    // bus -> ram
    input[31:0] dmem_read_data,         // dmem -> cpu
    output[31:0] dmem_write_data,       // cpu -> dmem
    output[31:0] dmem_addr,             // cpu -> dmem
    output reg dmem_wen,                // cpu -> dmem

    // bus -> rom
    input[31:0] dmem_rom_read_data,
    output[31:0] dmem_rom_addr,

    // bus -> uart
    input[31:0] uart_read_data,         // uart -> cpu
    output[31:0] uart_write_data,       // cpu -> uart
    output[31:0] uart_addr,             // cpu -> uart
    output reg uart_wen
);

    assign imem_addr = cpu_imem_addr;
    assign cpu_imem_data = imem_data;

    assign dmem_addr = cpu_dmem_addr;
    assign dmem_write_data = cpu_dmem_data_in;

    assign dmem_rom_addr = cpu_dmem_addr;

    assign uart_addr = cpu_dmem_addr;
    assign uart_write_data = cpu_dmem_data_in;

    always @(*) begin
        // CPU访问类型
        case (cpu_dmem_addr[31:28])
            // CPU 访问的是 ROM，把从 ROM 返回的数据赋给总线
            4'h0:begin  // ROM
                cpu_dmem_data_out <= dmem_rom_read_data;
                dmem_wen <= 0;
                uart_wen <= 0;
            end
            // CPU 访问的是 RAM，把 CPU 的写使能 cpu_dmem_wen 赋给 RAM 的写使能信号 dmem_wen
            // 同时把从 RAM 返回的数据赋给总线
            4'h1:begin  // RAM
                dmem_wen <= cpu_dmem_wen;
                cpu_dmem_data_out <= dmem_read_data;
                uart_wen <= 0;
            end
            // CPU 访问的是串行通信模块 UART，把 CPU 的写使能 cpu_dmem_wen 赋给 uart 的写使能信号 uart_wen
            // 同时把从 UART 返回的数据赋给总线
            4'h2:begin  // uart io
                uart_wen <= cpu_dmem_wen;
                cpu_dmem_data_out <= uart_read_data;
                dmem_wen <= 0;
            end
            default: begin
                dmem_wen <= 0;
                uart_wen <= 0;
                cpu_dmem_data_out <= 0;
            end
        endcase
    end
    
endmodule