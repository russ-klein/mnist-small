
`timescale 1ns/1ns

module tbench;

  reg CLK;
  reg HRESETn;

  wire [31:0]   S_ADDRESS;
  wire [3:0]    S_BL;
  wire          S_WE;
  wire          S_CE;
  wire [31:0]   S_WDATA;
  wire [31:0]   S_RDATA;
  wire [1:0]    S_TRANS;
  wire [1:0]    S_RESP;
  wire          S_READY;


  // tbx clkgen inactive_negedge
  initial begin
    CLK = 0;
    forever #50 CLK = ~CLK;
  end

  // tbx clkgen
  initial begin
    HRESETn = 1; 
    #100 HRESETn = 0;
    #10000 HRESETn = 1;
  end


  // DUT
  top top(
    .HCLK(CLK), 
    .PORESETn(HRESETn), 

    .TB_ADDRESS(S_ADDRESS),
    .TB_BL(S_BL),
    .TB_WE(S_WE),
    .TB_CE(S_CE),
    .TB_WDATA(S_WDATA),
    .TB_RDATA(S_RDATA),
    .TB_TRANS(S_TRANS),
    .TB_RESP(S_RESP),
    .TB_READY(S_READY)

  );


`ifdef CODELINK

  wire PORESETn;
  wire SYSRESETn;
  wire FCLK;
  wire HCLK;
  wire BIGEND;
  wire STKALIGNINIT;
  wire MPUDISABLE;
  wire FPUDISABLE;

  // Codelink Monitor
  codelink_cpu_CORTEXM4 monitor(
    .PORESETn       (PORESETn),
    .SYSRESETn      (SYSRESETn),
    .FCLK           (FCLK),
    .HCLK           (HCLK),
    .BIGEND         (BIGEND),
    .STKALIGNINIT   (STKALIGNINIT),
    .MPUDISABLE     (MPUDISABLE),
    .FPUDISABLE     (FPUDISABLE)
  );

`endif

`ifdef WARPCORE

  warpcore_synchronizer go_fast(CLK);

`endif

  // test finish
  wire [31:0] WADDR = tbench.top.HADDR[2];
  
  always @(posedge CLK) begin
    if (WADDR == 32'hFF000000 ) begin
      $display("TEST : PASS");
      $finish;
    end
    else if (WADDR == 32'hFF0000FF) begin
      $display("TEST : FAIL");
      $finish;
    end
  end

   sram #(
        .file_no     (0),
        .addr_bits   (22),
        .wait_states (0)
    ) tbench_memory (
        .clock       (CLK),
        .resetn      (HRESETn),
        .address     (S_ADDRESS),
        .trans       (S_TRANS),
        .bl          (S_BL),
        .we          (S_WE),
        .ce          (S_CE),
        .write_data  (S_WDATA),
        .read_data   (S_RDATA),
        .resp        (S_RESP),
        .ready       (S_READY)
   );

endmodule
