module cat_accel (
      input          clock,
      input          resetn,
      input  [1:0]   trans,
      input  [29:0]  address,
      input  [3:0]   bl,
      input          we,
      input          ce,
      input  [31:0]  write_data,
      output [31:0]  read_data,
      output [1:0]   resp,
      output         ready
  );

  parameter width         = 16;    // number of address bits
  parameter num_registers = 16;    // number of resgisters in bank

  reg  [31:0] register_bank[num_registers-1:0];
  reg  [31:0] rd_reg;

  reg  [15:0] write_address;
  wire [15:0] read_address;
  reg         write_enable;

  wire        ready_out; //  = 1'b1;
  reg         resp_out = 2'b00;   // OK

  ready_gen #(1) gr0 (clock, resetn, ce, trans, ready_out);

  always @(posedge clock or resetn == 1'b0) begin
    if (resetn == 1'b0) begin
      write_address <= 32'h00000000;
      write_enable  <= 1'b0;
    end else begin
      write_address <= address[15:0];
      write_enable  <= we & ce & trans[1];
    end
  end 

  assign ready     = ready_out;
  assign resp      = resp_out;
  assign read_data = rd_reg;
  assign read_address = address[15:0];

  always @(posedge clock or resetn == 1'b0) begin
    if (resetn == 1'b0) begin
      rd_reg <= 32'h00000000;
    end else begin
      if (trans[1] & ce) begin
        rd_reg <= register_bank[read_address];
      end
    end
  end

  always @(posedge clock) begin
    if (resetn == 1'b1) begin
      if (write_enable) begin
        if (write_address < num_registers) begin
          register_bank[write_address] <= write_data;
        end
      end
    end 
  end
endmodule
