module adder4(a, b, cin, sum, cout);
  input [3:0] a, b;   // 4位输入a和b
  input cin;          // 进位
  output [3:0] sum;   // 4位输出和
  output cout;        // 进位输出

  wire [3:0] c;       // 中间进位变量

  full_adder fa0(a[0], b[0], cin, sum[0], c[0]);  // 第一位的全加器
  full_adder fa1(a[1], b[1], c[0], sum[1], c[1]); // 第二位的全加器
  full_adder fa2(a[2], b[2], c[1], sum[2], c[2]); // 第三位的全加器
  full_adder fa3(a[3], b[3], c[2], sum[3], cout); // 第四位的全加器
endmodule

module full_adder(a, b, cin, sum, cout);
  input a, b, cin; // 输入a, b, 进位cin
  output sum, cout; // 输出和sum和进位cout

  assign sum = a ^ b ^ cin; // 异或运算
  assign cout = (a & b) | (a & cin) | (b & cin); // 与、或运算
endmodule
