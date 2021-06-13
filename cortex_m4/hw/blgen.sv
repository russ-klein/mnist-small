
`timescale 1ns/1ns

module blgen(hbl, size, addr);

    output [3:0] hbl;
    input [2:0] size;
    input [31:0] addr;

    reg [3:0] local_hbl;

    assign hbl = local_hbl;

    always @(size, addr) begin
       if (size == 3'b000) begin 
           if (addr[1:0] == 2'b00) local_hbl <= 4'b0001;
           if (addr[1:0] == 2'b01) local_hbl <= 4'b0010;
           if (addr[1:0] == 2'b10) local_hbl <= 4'b0100;
           if (addr[1:0] == 2'b11) local_hbl <= 4'b1000;
       end else if (size == 3'b001) begin 
           if (addr[1] == 1'b0)    local_hbl <= 4'b0011;
           if (addr[1] == 1'b1)    local_hbl <= 4'b1100;
       end else begin
                                   local_hbl <= 4'b1111;
       end 
    end 
endmodule
