module timer (
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

  reg [31:0]  timer_value;
  reg [31:0]  rd_reg;

  //reg         ready_out = 1'b1;
  reg         resp_out  = 2'b00;   

  ready_gen #(1) rg (clock, resetn, ce, trans, ready_out);

  assign ready          = ready_out;
  assign resp           = resp_out;
  assign read_data      = rd_reg;

  always @(posedge clock or negedge resetn) begin
    if (resetn == 1'b0) begin
      timer_value <= 32'h00000000;
    end else begin
      timer_value <= timer_value + 32'h00000001;
    end
  end
  
  always @(posedge clock or negedge resetn) begin
    if (resetn == 1'b0) begin
      rd_reg <= 32'h00000000;
    end else begin
      //if (trans[1] & ce) begin
      if (ce & !we) begin
        rd_reg <= timer_value;
      end
    end
  end

endmodule
