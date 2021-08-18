////////////////////////////////////////////////////////////////////////////////
// Catapult Synthesis
// 
// Copyright (c) 2003-2019 Calypto Design Systems, Inc.
//       All Rights Reserved
// 
// This document contains information that is proprietary to Calypto Design
// Systems, Inc. The original recipient of this document may duplicate this  
// document in whole or in part for internal business purposes only, provided  
// that this entire notice appears in all copies. In duplicating any part of  
// this document, the recipient agrees to make every reasonable effort to  
// prevent the unauthorized use and distribution of the proprietary information.
////////////////////////////////////////////////////////////////////////////////
// NO WARRANTY. CALYPTO DESIGN SYSTEMS INC. EXPRESSLY DISCLAIMS ALL WARRANTY 
// FOR THE SOFTWARE. TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE 
// LAW, THE SOFTWARE AND ANY RELATED DOCUMENTATION IS PROVIDED "AS IS"
// AND WITH ALL FAULTS AND WITHOUT WARRANTIES OR CONDITIONS OF ANY 
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, WITHOUT LIMITATION, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
// PURPOSE, OR NONINFRINGEMENT. THE ENTIRE RISK ARISING OUT OF USE OR 
// DISTRIBUTION OF THE SOFTWARE REMAINS WITH YOU.
////////////////////////////////////////////////////////////////////////////////

// --------------------------------------------------------------------------
// DESIGN UNIT:        ccs_ram_w_handshake
//
// DESCRIPTION:
//   This design unit models a generic memory using the internal catapult protol.
//   - Read and Write must be implemented for use in SCVerify
//   - Slave (to catapult) semantics.  The same protocol allows master memory implementation
//   - Supports burst, with latency for out-of-order addresses.
//   - Read and Write can happen concurrently
//   - Indeterminate of read/write to the same address at the same time
//
// CHANGE LOG:
//   02/20/2019 - Creation
//
//
// --------------------------------------------------------------------------
//`define DBG_READ 1
//`define DBG_WRITE 1

module ccs_ram_w_handshake (clk, rstn, 
                            s_re, s_rrdy, s_raddr, s_din,  // Read channel
                            s_we, s_wrdy, s_waddr, s_dout);  // Write channel
                            
   parameter   ram_id = 1;     // Resource ID
   parameter   depth = 16;     // Number of addressable elements (up to 20bit address)
   parameter   op_width = 1;   // dummy parameter for cwidth calculation
   parameter   width = 16;     // Catapult data bus width (multiples of 8)
   parameter   addr_w = 4;     // Catapult address bus width
   parameter   rst_ph = 0;     // Reset phase - negative default
   parameter   nopreload= 0;   // Transactor support
   
   input clk;
   input rstn;
   wire  int_rstn;
   assign int_rstn = rst_ph ? ~rstn : rstn;

   input                s_re;
   output               s_rrdy;
   input [addr_w-1 : 0] s_raddr;
   output [width-1:0]   s_din;

   input                s_we;
   output               s_wrdy;
   input [addr_w-1 : 0] s_waddr;
   input [width-1:0]    s_dout;
   
   reg [width-1:0]   s_din_reg;
   assign s_din = s_din_reg;
   
   reg [width-1:0]      memArray[depth-1:0];

   // Latency to mimic out-of-order cost for read channel data
   localparam latency=5;

   localparam [2:0] w_idle=0, w_write=1, w_stalling=2;
   localparam [2:0] r_idle=0, r_read=1, r_stalling=2;
   reg [1:0]            rState;
   reg [2:0]            wState;

   integer              dataSent;
   integer              rStartAddr;
   wire                 rOutOfOrder;
   integer              rStallCnt;

   integer              dataRcvd;
   integer              wStartAddr;
   wire                 wOutOfOrder;
   integer              wStallCnt;
   
   
   // Note - rOutOfOrder is glitchy right after the clock edge.  In delta-time
   assign rOutOfOrder = (rState == r_stalling) ||
                        ((rState == r_read) && 
                         ((s_raddr+1) != (rStartAddr + dataSent)) &&
                         int_rstn &&
                         s_re);
   
   reg                   s_rrdy_reg;
   assign s_rrdy = s_rrdy_reg && !rOutOfOrder;


   assign wOutOfOrder = (wState == w_stalling) ||
                        ((wState == w_write) && 
                         ((s_waddr) != (wStartAddr + dataRcvd)) &&
                         int_rstn &&
                         s_we);

   reg                   s_wrdy_reg;
   assign s_wrdy = s_wrdy_reg && !wOutOfOrder;
   
   always @(posedge clk or negedge int_rstn) begin
      if (~int_rstn) begin
         dataSent <= 0;
         rStartAddr <= 0;
         s_rrdy_reg <= 1;
         rState <= r_idle;
         rStallCnt <= 0;
      end else begin
`ifdef DBG_READ
         $display("rState=%d re=%b addr=%h start=%h dataSent=%d OOO=%b T=%t",
                  rState, s_re, s_raddr, rStartAddr, dataSent, rOutOfOrder, $time);
`endif
         if (rState == r_idle) begin
            if (s_re) begin
               s_din_reg <= memArray[s_raddr];
               s_rrdy_reg <= 1;
               rStartAddr <= s_raddr;
               dataSent <= 1;
               rState <= r_read;
`ifdef DBG_READ
               $display("  Drive Data=%h", memArray[s_raddr]);
`endif
            end else begin
               s_rrdy_reg <= 0;
`ifdef DBG_READ
               $display("  s_rrdy drives 0");
`endif
            end
         end else if (rState == r_read) begin
            // "burst" in progress
            if (rOutOfOrder) begin
`ifdef DBG_READ
               $display("  rOutOfOrder");
`endif
               if (latency == 0) begin
                  rState <= r_idle;
               end else begin
                  rState <= r_stalling;
               end
               rStallCnt <= 0;
               s_rrdy_reg <= 0;
               dataSent <= 0;
               rStartAddr <= 0;
            end else begin
               if (s_re) begin
                  if (s_raddr+1 >= depth) begin
`ifdef DBG_READ
                     $display("  rBursted to end of memory. Start over");
`endif
                     s_rrdy_reg <= 0;
                     dataSent <= 0;
                     rStartAddr <= 0;               
                     rState <= r_idle;
                  end else begin
                     s_din_reg <= memArray[s_raddr+1]; // predictive.  OutOfOrder protects us from the wrong guess
                     s_rrdy_reg <= 1;
                     dataSent <= dataSent+1;
`ifdef DBG_READ
                     $display("  Drive2 Data=%h", memArray[s_raddr+1]);
`endif
                  end
               end else begin
                  s_rrdy_reg <= 0;
                  dataSent <= 0;
                  rStartAddr <= 0;               
                  rState <= r_idle;
`ifdef DBG_READ
                  $display("  s_rrdy2 drives 0.  Start another burst");
`endif
               end
            end
         end else if (rState == r_stalling) begin
            if (rStallCnt >= latency) begin
`ifdef DBG_READ
               $display("  rStall finishes");
`endif
               rStallCnt <= 0;
               rState <= r_idle;
            end else begin
`ifdef DBG_READ
               $display("  Continue rStall");
`endif
               rStallCnt <= rStallCnt + 1;
            end
         end else begin
`ifdef DBG_READ
            $display("Error: Bad read state(%d) at T=%t", rState, $time);
`endif
            rState <= r_idle;
            s_rrdy_reg <= 0;
         end
      end
   end

   // Catapult write processing.

   always @(posedge clk or negedge int_rstn) begin
      if (~int_rstn) begin
         wState <= w_idle;
         wStartAddr<=0;
         s_wrdy_reg <= 1;
         wStallCnt <= 0;
         dataRcvd  <= 0;
      end else begin // if (~int_rstn)
`ifdef DBG_WRITE
         $display("wState=%d we=%b addr=%h start=%h dataRcvd=%d OOO=%b wrdy_reg=%b T=%t",
                  wState, s_we, s_waddr, wStartAddr, dataRcvd, wOutOfOrder, s_wrdy_reg, $time);
`endif

         if (wState == w_idle) begin
            if ((s_we == 1) && (s_wrdy_reg == 1)) begin  // both have to happen to accept the data
`ifdef DBG_WRITE
               $display("  write:mem[%d]=%h at T=%t", s_waddr, s_dout, $time);
`endif
               if (s_waddr >= depth) begin
                  $display("Error: Write address(%d)>memory depth(%d) at T=%t", s_waddr, depth, $time);
               end else begin
                  memArray[s_waddr] <= s_dout;
                  wStartAddr <= s_waddr;
                  dataRcvd <= 1;
                  wState <= w_write;
               end
            end else begin
               s_wrdy_reg <= 1;
            end

         end else if (wState == w_write) begin
            if (wOutOfOrder) begin
               // Need to "remember" the data/addr because rdy was asserted at last cycle
`ifdef DBG_WRITE
               $display("  wOutOfOrder.  mem[%d]=%h at T=%t", s_waddr, s_dout, $time);
`endif
               if (s_waddr >= depth) begin
                  $display("Error: Write address(%d)>memory depth(%d) at T=%t", s_waddr, depth, $time);
               end else begin
                  memArray[s_waddr] <= s_dout;
                  wState <= w_stalling;
                  wStallCnt <= 0;
                  s_wrdy_reg <= 0;  // No mas...
                  dataRcvd <= 0;
               end
            end else begin
               if (s_we) begin
                  if (s_waddr+1 >= depth) begin
`ifdef DBG_WRITE
                     $display("  wBursted to end of memory. Start over. mem[%d]=%h at T=%t", s_waddr, s_dout, $time);
`endif
                     memArray[s_waddr] <= s_dout;
                     s_wrdy_reg <= 0;
                     dataRcvd <= 0;
                     wStartAddr <= 0;               
                     wState <= w_idle;
                  end else begin
                     memArray[s_waddr] <= s_dout;
                     s_wrdy_reg <= 1;
                     dataRcvd <= dataRcvd+1;
`ifdef DBG_WRITE
                     $display("  write2:mem[%d]=%h at T=%t", s_waddr, s_dout, $time);
`endif
                  end 
               end else begin 
                  // Start another burst
                  s_wrdy_reg <= 1;
                  dataRcvd <= 0;
                  wStartAddr <= 0;
                  wState <= w_idle;
`ifdef DBG_WRITE
                  $display("  s_we not asserted - start next burst");
`endif
                  
               end
            end
         end else if (wState == w_stalling) begin
            if (wStallCnt >= latency) begin
`ifdef DBG_WRITE
               $display("  wStall finishes");
`endif
               wStallCnt <= 0;
               wState <= w_idle;
               s_wrdy_reg <= 1;
            end else begin
`ifdef DBG_WRITE
               $display("  Continue wStall");
`endif
               wStallCnt <= wStallCnt + 1;
            end
         end else begin
`ifdef DBG_WRITE
            $display("Error: Bad write state(%d) at T=%t", wState, $time);
`endif
            wState <= w_idle;
            s_wrdy_reg <= 0;
         end
      end
   end

   
endmodule

