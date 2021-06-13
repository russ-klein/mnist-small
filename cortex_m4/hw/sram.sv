`timescale 1ns/1ns

`ifdef SIMULATION
`define deasserted 1'bz
`else
`define deasserted 1'b0
`endif

module ready_gen(
    input hclk,
    input hresetn,
    input ce,
    input [1:0] htrans,
    output hready
);

    parameter waits = 3;
    reg [7:0] count;
    reg am_waiting;
    reg lhready;
    
    assign hready = lhready;

    always @(posedge hclk or negedge hresetn) begin
       if (hresetn == 0) begin
          count <= 1'b0;
          am_waiting <= 1'b0;
          lhready <= 1'b0;
       end else begin
          if (ce) begin
             if (am_waiting) begin
                count <= count + 1;
             end else begin
                am_waiting <= 1'b1;
                count <= 8'h00;
             end
             if (count >= waits) begin
                lhready <= 1'b1;
                am_waiting <= 1'b0;
             end
          end
          if (lhready) lhready <= 1'b0;
       end
   end
endmodule
           


/*
    reg [7:0]count;
    reg lhready;
    reg am_waiting;
    
    always @(posedge hclk or negedge hresetn)
    if (hresetn == 0) begin
        lhready <= 1'b0;
        count <= 8'h0;
        am_waiting <= 1'b0;
    end else begin
        if (htrans[1] == 1'b1) begin // start of transaction
            if (waits == 8'b0) begin
                lhready <= 1'b1;
            end else begin
                if (count == 8'h0) begin
                    if (am_waiting == 1'b1) begin
                        am_waiting <= 1'b0;
                        lhready <= 1'b1;
                    end else begin
                        am_waiting <= 1'b1;
                        lhready <= 1'b0;
                        count <= waits;
                    end;
                end else begin
                    lhready <= 1'b0;
                    count <= count - 8'h1;
                end
            end
        end else begin
            lhready <= 1'b0;
        end
    end

    assign hready = lhready;
endmodule

module byte_sram (
      input         clock,
      input  [29:0] address,
      input         we,
      input         ce,
      input  [7:0]  write_data,
      output [7:0]  read_data
  );

  parameter width = 16;

  reg [7:0] mem [(1<<width)-1:0];
  reg [width-1:0] addr;
  reg [7:0] data_out;
  reg [width-1:0] write_addr;
  reg write_strobe = 1'b0;

  assign addr = address[width-1:0];
  assign read_data = data_out;
  assign ws = we & ce;

  always @(posedge clock) begin
    write_strobe <= ws;
  end

  always @(posedge clock) begin
    if (we && ce) begin
      write_addr <= addr; 
    end
    if (!we && ce) begin
      data_out <= mem[addr];
//$display("memory read ", data_out);
    end
    if (write_strobe) begin
      mem[write_addr] <= write_data;
    end
  end

endmodule

*/
module long_sram
    (
        CLK,
        READ_ADDR,
        DATA_OUT,
        OE,
        WRITE_ADDR_RAW,
        DATA_IN,
        BE_RAW,
        WE_RAW
    );

    parameter file_no           = 0;
    parameter address_width     = 22;
    parameter data_width        = 2; //   in 2^data_width bytes
                                     //   0 = 8 bits  1 = 16 bits  2 = 32 bits  3 = 64 bits
                                     //   memory is always byte addressible

    input                                CLK;
    input [address_width-1:0]            READ_ADDR;
    output [((1<<data_width)*8)-1:0]     DATA_OUT;
    input                                OE;
    input [address_width-1:0]            WRITE_ADDR_RAW;
    input [((1<<data_width)*8)-1:0]      DATA_IN;
    input [(1<<data_width)-1:0]          BE_RAW;
    input                                WE_RAW;


    reg [((1<<data_width)*8)-1:0] mem [(1<<address_width-data_width)-1:0];
    reg [((1<<data_width)*8)-1:0] read_data;

    reg [address_width-1:0]       WRITE_ADDR;
    reg                           WE;
    reg [(1<<data_width)-1:0]     BE;

    genvar w;

    initial begin
        if (file_no == 1) $readmemh("code.mem", mem);
        if (file_no == 2) $readmemh("weights.mem", mem);
    end

    assign DATA_OUT = read_data;

    always @(posedge CLK) begin
        WRITE_ADDR <= WRITE_ADDR_RAW;
        WE <= WE_RAW;
        BE <= BE_RAW;
    end

    always @(posedge CLK) begin
        if (OE) begin
            read_data <= ((WE === 1'b1) && (READ_ADDR == WRITE_ADDR)) ? DATA_IN : mem[READ_ADDR];
            //$display("read %08x at %08x ", mem[READ_ADDR], READ_ADDR*4);
        end
    end
/*
    always @(posedge CLK) begin
      if (WE) begin 
        if (BE == 4'b1111) $display("wrote %08x at %08x ", DATA_IN, 4*WRITE_ADDR);
        if (BE == 4'b0011) $display("wrote     %04x at %08x ", DATA_IN, 4*WRITE_ADDR);
        if (BE == 4'b1100) $display("wrote     %04x at %08x ", DATA_IN, 4*WRITE_ADDR+2);
        if (BE == 4'b0001) $display("wrote       %02x at %08x ", DATA_IN, 4*WRITE_ADDR);
        if (BE == 4'b0010) $display("wrote       %02x at %08x ", DATA_IN, 4*WRITE_ADDR+1);
        if (BE == 4'b0100) $display("wrote       %02x at %08x ", DATA_IN, 4*WRITE_ADDR+2);
        if (BE == 4'b1000) $display("wrote       %02x at %08x ", DATA_IN, 4*WRITE_ADDR+3);
      end
    end
*/

    generate
        for (w=0; w<(1<<data_width); w = w + 1)
            always @(posedge CLK) begin
                if (WE && BE[w]) begin
                    mem[WRITE_ADDR][((w+1)*8)-1:(w*8)] <= DATA_IN[((w+1)*8)-1:(w*8)];
                end
            end
    endgenerate

endmodule


module sram (
      input          clock,
      input          resetn,
      input  [1:0]   trans,
      input  [31:0]  address,
      input  [3:0]   bl,
      input          we,
      input          ce,
      input  [31:0]  write_data,
      output [31:0]  read_data,
      output [1:0]   resp,
      output         ready
  );

  parameter file_no     = 0;
  parameter addr_bits   = 16;    // number of address bits (default 64K)
  parameter wait_states = 0;

  reg [31:0] data_out;
  wire ready_out;
  reg resp_out = 2'b00;  // OK
  reg [31:0] waddr_delayed;
  wire [31:0] mem_address;
  wire [3:0] web;
  reg we_delayed;
  reg ce_delayed;
  reg [3:0] bl_delayed;

  assign ready = ready_out;
  //assign ready = (!wait_states) ? 1'b1 : ready_out;
  assign resp = resp_out;

  ready_gen #(wait_states) gr0 (clock, resetn, ce, trans, ready_out);


  always @(posedge clock) begin
     we_delayed <= (ce) ? we : 1'b0;
     ce_delayed <= ce;
     bl_delayed <= (ce) ? bl : 4'h0;
     waddr_delayed <= (ce) ? address : 32'h00000000;
  end

  assign web = { we_delayed & ce_delayed & bl_delayed[3] & ready_out, 
                 we_delayed & ce_delayed & bl_delayed[2] & ready_out, 
                 we_delayed & ce_delayed & bl_delayed[1] & ready_out, 
                 we_delayed & ce_delayed & bl_delayed[0] & ready_out };
 
  assign mem_address = (we_delayed) ? waddr_delayed : address;

  //chatty_long_sram #(.file_no(file_no), .address_width(addr_bits)) mem (clock, address[addr_bits+1:2], data_out, ce & !we, address[addr_bits+1:2], write_data, bl, we & ce, ready_out);
  //long_sram #(.file_no(file_no), .address_width(addr_bits)) mem(clock, mem_address[addr_bits+1:2], data_out, ce & !we, write_data, bl_delayed, we_delayed & ce_delayed & ready_out);
  byte_sram #(.address_width(addr_bits-2)) 
         byte0 (clock, mem_address[addr_bits-1:2], data_out[7:0], ce & !we, write_data[7:0], web[0]);

  byte_sram #(.address_width(addr_bits-2)) 
         byte1 (clock, mem_address[addr_bits-1:2], data_out[15:8], ce & !we, write_data[15:8], web[1]);

  byte_sram #(.address_width(addr_bits-2)) 
         byte2 (clock, mem_address[addr_bits-1:2], data_out[23:16], ce & !we, write_data[23:16], web[2]);

  byte_sram #(.address_width(addr_bits-2)) 
         byte3 (clock, mem_address[addr_bits-1:2], data_out[31:24], ce & !we, write_data[31:24], web[3]);

  assign read_data = (ready & !we_delayed) ? data_out : { 32 { `deasserted}}; 

endmodule


module byte_sram
    (
        CLK,
        ADDRESS,
        DATA_OUT,
        OE,
        DATA_IN,
        WE
    );

    parameter address_width       = 14;

    input                         CLK;
    input [address_width-1:0]     ADDRESS;
    output [7:0]                  DATA_OUT;
    input                         OE;
    input  [7:0]                  DATA_IN;
    input                         WE;

    reg [7:0] mem [(1<<address_width)-1:0];
    reg [7:0] read_data;

    assign DATA_OUT = read_data;

    always @(posedge CLK) begin
        if (OE) begin
            read_data <= mem[ADDRESS];
        end
    end

    always @(posedge CLK) begin
        if (WE) begin  
            mem[ADDRESS] <= DATA_IN;
            //$display("%8t wrote: %02x at %06x ", $time, DATA_IN, ADDRESS<<2);
        end
    end

endmodule


module long_sram_1r1w
    (
        CLK,
        ADDRESS,
        DATA_OUT,
        OE,
        DATA_IN,
        BE,
        WE
    );

    parameter file_no           = 0;
    parameter address_width     = 22;
    parameter data_width        = 2; //   in 2^data_width bytes
                                     //   0 = 8 bits  1 = 16 bits  2 = 32 bits  3 = 64 bits
                                     //   memory is always byte addressible

    input                                CLK;
    input [address_width-1:0]            ADDRESS;
    output [((1<<data_width)*8)-1:0]     DATA_OUT;
    input                                OE;
    input [((1<<data_width)*8)-1:0]      DATA_IN;
    input [(1<<data_width)-1:0]          BE;
    input                                WE;

    reg [((1<<data_width)*8)-1:0] mem [(1<<address_width-data_width)-1:0];
    reg [((1<<data_width)*8)-1:0] read_data;

    genvar w;

    initial begin
        if (file_no == 1) $readmemh("code.mem", mem);
        if (file_no == 2) $readmemh("weights.mem", mem);
    end

    assign DATA_OUT = read_data ;

    always @(posedge CLK) begin
        if (OE) begin
            read_data <= mem[ADDRESS];
            // $display("(%10t) >>>     MEM: read %08x at %08x ", $time, mem[ADDRESS], ADDRESS*4);
        end
    end
/*
    always @(posedge CLK) begin
      if (WE) begin 
        if (BE == 4'b1111) $display("(%10t) >>> MEM: wrote     %08x at %08x ", $time, DATA_IN, 4*ADDRESS);
        if (BE == 4'b0011) $display("(%10t) >>> MEM: wrote         %04x at %08x ", $time, DATA_IN, 4*ADDRESS);
        if (BE == 4'b1100) $display("(%10t) >>> MEM: wrote         %04x at %08x ", $time, DATA_IN, 4*ADDRESS+2);
        if (BE == 4'b0001) $display("(%10t) >>> MEM: wrote           %02x at %08x ", $time, DATA_IN, 4*ADDRESS);
        if (BE == 4'b0010) $display("(%10t) >>> MEM: wrote           %02x at %08x ", $time, DATA_IN, 4*ADDRESS+1);
        if (BE == 4'b0100) $display("(%10t) >>> MEM: wrote           %02x at %08x ", $time, DATA_IN, 4*ADDRESS+2);
        if (BE == 4'b1000) $display("(%10t) >>> MEM: wrote           %02x at %08x ", $time, DATA_IN, 4*ADDRESS+3);
      end
    end
*/

    generate
        for (w=0; w<(1<<data_width); w = w + 1)
            always @(posedge CLK) begin
                if (WE && BE[w]) begin  
                    mem[ADDRESS][((w+1)*8)-1:(w*8)] <= DATA_IN[((w+1)*8)-1:(w*8)];
                end
            end
    endgenerate
endmodule

module chatty_long_sram
    (
        CLK,
        READ_ADDR,
        DATA_OUT,
        OE,
        WRITE_ADDR_RAW,
        DATA_IN,
        BE_RAW,
        WE_RAW,
        STROBE
    );

    parameter file_no           = 0;
    parameter address_width     = 22;
    parameter data_width        = 2; //   in 2^data_width bytes
                                     //   0 = 8 bits  1 = 16 bits  2 = 32 bits  3 = 64 bits
                                     //   memory is always byte addressible

    input                                CLK;
    input [address_width-1:0]            READ_ADDR;
    output [((1<<data_width)*8)-1:0]     DATA_OUT;
    input                                OE;
    input [address_width-1:0]            WRITE_ADDR_RAW;
    input [((1<<data_width)*8)-1:0]      DATA_IN;
    input [(1<<data_width)-1:0]          BE_RAW;
    input                                WE_RAW;
    input                                STROBE;


    reg [((1<<data_width)*8)-1:0] mem [(1<<address_width-data_width)-1:0];
    reg [((1<<data_width)*8)-1:0] read_data;

    reg [address_width-1:0]       WRITE_ADDR;
    reg                           WE;
    reg [(1<<data_width)-1:0]     BE;

    genvar w;

    initial begin
        if (file_no == 1) $readmemh("code.mem", mem);
        if (file_no == 2) $readmemh("weights.mem", mem);
    end

    assign DATA_OUT = read_data ;

    always @(posedge CLK) begin
        WRITE_ADDR <= WRITE_ADDR_RAW;
        WE <= WE_RAW;
        BE <= BE_RAW;
    end

    always @(posedge CLK) begin
        if (OE) begin
            read_data <= ((WE === 1'b1) && (READ_ADDR == WRITE_ADDR)) ? DATA_IN : mem[READ_ADDR];
            // $display("(%10t) >>>     MEM: read %08x at %08x ", $time, mem[READ_ADDR], READ_ADDR*4);
        end
    end
/*
    always @(posedge CLK) begin
      if (WE) begin 
        if (BE == 4'b1111) $display("(%10t) >>> MEM: wrote     %08x at %08x ", $time, DATA_IN, 4*WRITE_ADDR);
        if (BE == 4'b0011) $display("(%10t) >>> MEM: wrote         %04x at %08x ", $time, DATA_IN, 4*WRITE_ADDR);
        if (BE == 4'b1100) $display("(%10t) >>> MEM: wrote         %04x at %08x ", $time, DATA_IN, 4*WRITE_ADDR+2);
        if (BE == 4'b0001) $display("(%10t) >>> MEM: wrote           %02x at %08x ", $time, DATA_IN, 4*WRITE_ADDR);
        if (BE == 4'b0010) $display("(%10t) >>> MEM: wrote           %02x at %08x ", $time, DATA_IN, 4*WRITE_ADDR+1);
        if (BE == 4'b0100) $display("(%10t) >>> MEM: wrote           %02x at %08x ", $time, DATA_IN, 4*WRITE_ADDR+2);
        if (BE == 4'b1000) $display("(%10t) >>> MEM: wrote           %02x at %08x ", $time, DATA_IN, 4*WRITE_ADDR+3);
      end
    end
*/

    generate
        for (w=0; w<(1<<data_width); w = w + 1)
            always @(posedge CLK) begin
                if (WE && BE[w] & STROBE) begin  
                    mem[WRITE_ADDR][((w+1)*8)-1:(w*8)] <= DATA_IN[((w+1)*8)-1:(w*8)];
                end
            end
    endgenerate

endmodule


module chatty_sram (
      input          clock,
      input          resetn,
      input  [1:0]   trans,
      input  [31:0]  address,
      input  [3:0]   bl,
      input          we,
      input          ce,
      input  [31:0]  write_data,
      output [31:0]  read_data,
      output [1:0]   resp,
      output         ready
  );

  parameter width = 16;    // number of address bits (default 64K)
  parameter wait_states = 0;

  wire ready_out;
  reg resp_out = 2'b00;  // OK

  assign ready = (!wait_states) ? 1'b1 : ready_out;
  assign resp = resp_out;

  ready_gen #(wait_states) gr0 (clock, resetn, trans, ready_out);

  chatty_long_sram #(22) mem (clock, address[23:2], read_data, ce & !we, address[23:2], write_data, bl, we & ce);

endmodule

