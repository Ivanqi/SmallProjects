module adder4_tb;
  reg [3:0] a;
  reg [3:0] b;
  reg cin;
  wire [3:0] sum;
  wire cout;

  adder4 adder(a, b, cin, sum, cout);

  initial begin
    // 测试1：a=4'b0000, b=4'b0000, cin=0
    a = 4'b0000;
    b = 4'b0000;
    cin = 0;
    #10;
    $display("a=%b, b=%b, sum=%b, cout=%b", a, b, sum, cout);

    // 测试2：a=4'b1111, b=4'b1111, cin=0
    a = 4'b1111;
    b = 4'b1111;
    cin = 0;
    #10;
    $display("a=%b, b=%b, sum=%b, cout=%b", a, b, sum, cout);

    // 测试3：a=4'b1100, b=4'b1010, cin=1
    a = 4'b1100;
    b = 4'b1010;
    cin = 1;
    #10;
    $display("a=%b, b=%b, sum=%b, cout=%b", a, b, sum, cout);
  end
endmodule
