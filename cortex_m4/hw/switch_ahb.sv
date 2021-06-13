
`timescale 1ns/1ns

`ifdef SIMULATION
`define deasserted 1'bz
`else
`define deasserted 1'b0
`endif

module transparent_latch(
          HCLOCK, 
          HRESETn,
               
          LATCH_OUT,
          HREADY,
          HTRANS_IS_NOT_0,
          LATCH_IN
   );

   input  HCLOCK;
   input  HRESETn;
   output LATCH_OUT;
   input  HREADY;
   input  HTRANS_IS_NOT_0;
   input  LATCH_IN;

   reg    latched_value;
   

   always @(posedge HCLOCK or negedge HRESETn) begin
      if (HRESETn == 0) begin
         latched_value <= `deasserted;
      end else begin
         if (HREADY) 
            if (HTRANS_IS_NOT_0) latched_value <= LATCH_IN;
            else                 latched_value <= `deasserted;
      end
   end

   assign LATCH_OUT = (HREADY) ? ((HTRANS_IS_NOT_0) ? LATCH_IN : `deasserted) :  latched_value;
   //assign LATCH_OUT = (HREADY) ? LATCH_IN : latched_value;
endmodule          


module transparent_bus_latch(
          HCLOCK, 
          HRESETn,
               
          LATCH_OUT,
          HREADY,
          HTRANS_IS_NOT_0,
          LATCH_IN
   );

   parameter width = 32;

   input              HCLOCK;
   input              HRESETn;
   output [width-1:0] LATCH_OUT;
   input              HREADY;
   input              HTRANS_IS_NOT_0;
   input  [width-1:0] LATCH_IN;

   reg    [width-1:0] latched_value;

   always @(posedge HCLOCK or negedge HRESETn) begin
      if (HRESETn == 0) begin
         latched_value <= {width{`deasserted}};
      end else begin
         if (HREADY) 
            if (HTRANS_IS_NOT_0) latched_value <= LATCH_IN;
            else                 latched_value <= {width{`deasserted}};
      end
   end

   assign LATCH_OUT = (HREADY) ? ((HTRANS_IS_NOT_0) ? LATCH_IN : {width{`deasserted}}) :  latched_value;
   //assign LATCH_OUT = (HREADY) ? LATCH_IN : latched_value;
endmodule


module latches_valid(
          HCLOCK, 
          HRESETn,
               
          LATCHES_VALID,
          HREADY,
          HTRANS_IS_NOT_0
   );

   input  HCLOCK;
   input  HRESETn;
   output LATCHES_VALID;
   input  HREADY;
   input  HTRANS_IS_NOT_0;

   reg    lv;

   always @(posedge HCLOCK or negedge HRESETn) begin
      if (HRESETn == 0) begin
         lv <= 0;
      end else begin
         if (HREADY) 
            if (HTRANS_IS_NOT_0) lv <= 1;
            else                 lv <= 0;
      end
   end

   assign LATCHES_VALID = (HREADY) ? HTRANS_IS_NOT_0 : lv;

endmodule

/*
module transparent_latch(
          HCLOCK, 
          HRESETn,
               
          LATCH_OUT,
          HREADY,
          HTRANS_IS_0,
          LATCH_IN
   );

   input  HCLOCK;
   input  HRESETn;
   output LATCH_OUT;
   input  ENABLE;
   input  LATCH_IN;

   reg    latched_value;

   always @(posedge HCLOCK or negedge HRESETn) begin
      if (HRESETn == 0) begin
         latched_value <= 0;
      end else begin
         if (ENABLE) latched_value <= LATCH_IN;
      end
   end

   assign LATCH_OUT = (ENABLE) ? LATCH_IN : latched_value;
endmodule          


module transparent_bus_latch(
          HCLOCK, 
          HRESETn,
               
          LATCH_OUT,
          ENABLE,
          LATCH_IN
   );

   parameter width = 32;

   input              HCLOCK;
   input              HRESETn;
   output [width-1:0] LATCH_OUT;
   input              ENABLE;
   input  [width-1:0] LATCH_IN;

   reg    [width-1:0] latched_value;

   always @(posedge HCLOCK or negedge HRESETn) begin
      if (HRESETn == 0) begin
         latched_value <= {width{1'b0}};
      end else begin
         if (ENABLE) latched_value <= LATCH_IN;
      end
   end

   assign LATCH_OUT = (ENABLE) ? LATCH_IN : latched_value;
endmodule
*/


module ahb_switch(

          HCLOCK, 
          HRESETn,

          // AHB master signals 

          HADDR,
          HTRANS,
          HBURST,
          HWRITE,
          HSIZE,
          HPROT,
          HREADY,
          HRESP,
          HWDATA,
          HRDATA,
          HLOCK,

          // AHB Slave signals 

          S_ADDRESS,
          S_CHIP_SELECT,
          S_BYTE_ENABLE,
          S_WRITE,
          S_WDATA,
          S_TRANS,
          S_RDATA,
          S_RESP,
          S_READY
   );

   parameter masters  = 3;
   parameter slaves   = 2;
   parameter channels = 1;

   input             HCLOCK; 
   input             HRESETn;

   // AHB master signals 

   input  [31:0]     HADDR[masters-1:0];
   input  [1:0]      HTRANS[masters-1:0];
   input  [2:0]      HBURST[masters-1:0];
   input             HWRITE[masters-1:0];
   input  [2:0]      HSIZE[masters-1:0];
   input  [3:0]      HPROT[masters-1:0];
   output            HREADY[masters-1:0];
   output [1:0]      HRESP[masters-1:0];
   input  [31:0]     HWDATA[masters-1:0];
   output [31:0]     HRDATA[masters-1:0];
   input             HLOCK[masters-1:0];

   // AHB Slave signals 
 
   output [31:0]     S_ADDRESS[slaves-1:0];
   output            S_CHIP_SELECT[slaves-1:0];
   output [3:0]      S_BYTE_ENABLE[slaves-1:0];
   output            S_WRITE[slaves-1:0];
   output [31:0]     S_WDATA[slaves-1:0];
   output [1:0]      S_TRANS[slaves-1:0];
   input  [31:0]     S_RDATA[slaves-1:0];
   input  [1:0]      S_RESP[slaves-1:0];
   input             S_READY[slaves-1:0];

   reg  [3:0]        slave_table [masters-1:0];
   reg  [3:0]        slave_table_d [masters-1:0];
   reg  [3:0]        master_table [slaves-1:0];
   reg               master_valid [slaves-1:0];

   reg               hready               [masters-1:0];
   reg               hready_delayed       [masters-1:0];
   reg               hwrite_delayed       [masters-1:0];
   reg  [1:0]        htrans_delayed       [masters-1:0];
   reg  [31:0]       latched_hwdata       [masters-1:0];

   reg  [3:0]        target               [masters-1:0];
   reg  [3:0]        target_delayed       [masters-1:0];
   reg  [3:0]        target_data_phase    [masters-1:0];
   reg               grants               [masters-1:0];
   reg               grant_regs           [masters-1:0];
   reg               grant_combos         [masters-1:0];
   reg               grants_delayed       [masters-1:0];
   reg               grants_data_phase    [masters-1:0];
   reg               requests     [slaves-1:0][masters-1:0];
   reg               grant_array  [slaves-1:0][masters-1:0];
   reg               address_phase_active [masters-1:0];
   reg               data_phase_active    [masters-1:0];
   reg               addr_phase_active    [masters-1:0];
   reg               m_connected_a        [masters-1:0];
   reg               m_connected_d        [masters-1:0];
   reg               req_vector           [masters-1:0][masters-1:0];
   reg               goose                [masters-1:0];
   
   reg               a[masters-1:0];
   reg               b[masters-1:0];
   reg               c[masters-1:0];

   reg               latched_values_valid [masters-1:0];
   reg  [31:0]       latched_haddr        [masters-1:0];
   reg  [1:0]        latched_htrans       [masters-1:0];
   reg  [2:0]        latched_hburst       [masters-1:0];
   reg               latched_hwrite       [masters-1:0];
   reg  [2:0]        latched_hsize        [masters-1:0];
   reg  [3:0]        latched_hprot        [masters-1:0];
   reg               latched_hlock        [masters-1:0];
   reg  [3:0]        latched_byte_lanes   [masters-1:0];

   reg  [31:0]       latched_haddr2       [masters-1:0];
   reg  [31:0]       alt_latched_haddr2   [masters-1:0];

   reg               slave_busy           [slaves-1:0];
   reg               slave_busy_delayed   [slaves-1:0];
   reg               slave_busy_data_phase   [slaves-1:0];
   reg  [3:0]        connected_address_phase[slaves-1:0];
   reg  [3:0]        connected_address_phase_delayed[slaves-1:0];
   reg  [3:0]        connected_data_phase [slaves-1:0];
   reg  [3:0]        latched_connected_data_phase [slaves-1:0];


   // address mapping for peripherals

   function [3:0] slave_select;
     input [31:0] addr;

     begin
`ifdef TB_MEM
       if (addr[31:28] == 4'h0) return 0;
       if (addr[31:28] == 4'h4) return 5;
       if (addr[31:28] == 4'h8) return 2;
       if (addr[31:28] == 4'hA) return 3;
       if (addr[31:28] == 4'h9) return 4;
       if (addr[31:28] == 4'hD) return 1;
       return 15;
`else
       if (addr[31:28] == 4'h0) return 0;
       if (addr[31:28] == 4'h4) return 1;
       if (addr[31:28] == 4'h8) return 2;
       if (addr[31:28] == 4'hA) return 3;
       if (addr[31:28] == 4'h9) return 4;
       if (addr[31:28] == 4'hD) return 5;
       return 15;
`endif
     end
   endfunction

   function [3:0] oh_to_idx;  // one hot to index, 16 input 4 output bits
     input one_hot_in [masters-1:0];

     logic [15:0] one_hot; 
     logic [3:0] result;
     integer i;

     for (i=0; i<16; i++) if (i<masters) one_hot[i] = one_hot_in[i]; else one_hot[i] = 1'b0;
     
     result[0] = one_hot[15] | one_hot[13] | one_hot[11] | one_hot[9] | one_hot[7] | one_hot[5] | one_hot[3] | one_hot[1];
     result[1] = one_hot[15] | one_hot[14] | one_hot[11] | one_hot[10]| one_hot[7] | one_hot[6] | one_hot[3] | one_hot[2];
     result[2] = one_hot[15] | one_hot[14] | one_hot[13] | one_hot[12]| one_hot[7] | one_hot[6] | one_hot[5] | one_hot[4];
     result[3] = one_hot[15] | one_hot[14] | one_hot[13] | one_hot[12]| one_hot[11]| one_hot[10]| one_hot[9] | one_hot[8];
     return result;
   endfunction


   function any_bit_set;
      input one_hot [masters-1:0];
      integer i;

      for (i=0; i<masters; i++) if (one_hot[i]) return 1'b1;
      return 1'b0;
   endfunction
/*
   typedef logic grant_type[] ;
   function grant_type first_one ;
      input bits [masters-1:0];
      logic bits_out [masters-1:0];
      integer i;

      for (i=0; i<masters; i++) bits_out[i] = 1'b0;
      for (i=0; i<masters; i++) begin
         if (bits[i]) begin 
            bits_out[i] = 1'b1; 
            return bits_out; 
         end
      end

      return bits_out;

   endfunction
*/

   function first_one;
      input bits [masters-1:0];
      input n;
      integer i;

      for (i=0; i<n; i++) if (bits[i]) return 1'b0;
      return 1'b1;
   endfunction

   genvar m;
   genvar s;


   generate
      for (m=0; m<masters; m++) begin
         for (s=0; s<slaves; s++) begin
            assign requests[s][m] = (s == target[m]) & latched_values_valid[m];
         end
      end
   endgenerate

   generate
      for (m=0; m<masters; m++) begin

         always @(posedge HCLOCK or negedge HRESETn) begin
             if (HRESETn == 1'b0) begin
                 data_phase_active[m] <= 1'b0;
             end else begin
                 data_phase_active[m] <= address_phase_active[m];
             end
         end
/*
         assign HREADY[m] = hready[m];

         always @(posedge HCLOCK or negedge HRESETn) begin
             if (HRESETn == 1'b0) begin
                 hready[m] <= 1'b0;
             end else begin
                 if (HTRANS[m] == 2'b00) begin 
                    hready[m] <= 1'b1;
                 end else begin 
                    hready[m] <= S_READY[target[m]];
                 end
             end
         end
*/
         //assign HREADY[m] = grants[m] ? S_READY[target_delayed[m]] : (HTRANS[m] == 2'b00);
         //assign HREADY[m] = (!data_phase_active[m]) ? 1'b1 : grants[m] ? S_READY[target_delayed[m]] : (HTRANS[m] == 2'b00);
         //assign HREADY[m] = (!data_phase_active[m]) ? 1'b1 : grants[m] ? S_READY[target_delayed[m]] : 1'b0;
/*
         assign a[m] = (!grants[m] && !grants_data_phase[m]);
         assign b[m] = ( grants[m] && !grants_data_phase[m]);
         assign c[m] = (               grants_data_phase[m]);

         assign HREADY[m] = (!grants[m] && !grants_data_phase[m]) ? (HTRANS[m] == 2'b00) :
                            ( grants[m] && !grants_data_phase[m]) ? 1'b1 :
                            (               grants_data_phase[m]) ? S_READY[target_delayed[m]] : `deasserted;

         assign a[m] = (!grants[m] && !grants_delayed[m]);
         assign b[m] = ( grants[m] && !grants_delayed[m]);
         assign c[m] = (               grants_delayed[m]);


         assign HREADY[m] = (!grants[m] && !grants_delayed[m]) ? 1'b1 : // was this -> (HTRANS[m] == 2'b00) :
                            ( grants[m] && !grants_delayed[m]) ? 1'b1 :
                            (               grants_delayed[m]) ? S_READY[target_delayed[m]] : `deasserted;
*/

         always @(posedge HCLOCK or negedge HRESETn) begin
            if (HRESETn == 1'b0) begin
               goose[m] <= 1'b0;
            end else begin
               if ((HREADY[m] == 1'b0) && (HTRANS[m] != 2'b00) && !data_phase_active[m]) begin
                  goose[m] <= 1'b1;
               end 
               if (goose[m]) goose[m] <= 1'b0;
            end
         end 
     
         assign HREADY[m] = goose[m] | ((data_phase_active[m] & grants[m]) ? S_READY[target_delayed[m]] : 1'b0);
                            // ( grants[m]           )               ? grants[m] : (HTRANS[m] != 2'b00);

         //assign HRDATA[m] = grants_delayed[m] ? S_RDATA[target_data_phase[m]] : {32 {`deasserted}};
         //assign HRESP[m]  = grants_delayed[m] ? S_RESP[target_data_phase[m]] : 2'b0;
         assign HRDATA[m] = grants_delayed[m] ? S_RDATA[target_delayed[m]] : {32 {`deasserted}};
         assign HRESP[m]  = grants_delayed[m] ? S_RESP[target_delayed[m]] : 2'b0;

         //assign target[m] = slave_select(HADDR[m]);
         assign target[m] = latched_values_valid[m] ? slave_select(latched_haddr[m]) : 4'b0;

         always @(posedge HCLOCK or negedge HRESETn) begin
            if (HRESETn == 1'b0) begin
               target_data_phase[m] <= 4'h0;
               grants_data_phase[m] <= 1'b0;
               target_delayed[m] <= 1'b0;
               grants_delayed[m] <= 1'b0;
            end else begin
               target_delayed[m] <= target[m];
               grants_delayed[m] <= grants[m];   
               if (HREADY[m]) begin
                   target_data_phase[m] <= target[m];
                   grants_data_phase[m] <= grants[m];
               end
            end
         end

         assign address_phase_active[m] = latched_values_valid[m]; //  && grants[m];
 
         always @(posedge HCLOCK or negedge HRESETn) begin
            if (HRESETn == 1'b0) begin
               grants[m] <= 1'b0;
            end else begin
               if (requests[target[m]][m]) begin
                  if (!slave_busy[target[m]]) begin  // or grab "first one"
                     grants[m] <= first_one(requests[target[m]], m);
                  end
               end
               
               if (grants[m] & HREADY[m] & (HTRANS[m] == 2'b00)) grants[m] <= 1'b0; 
               //if (HTRANS[m] == 2'b00) grants[m] <= 1'b0;
            end
         end

/*
         always @(posedge HCLOCK or negedge HRESETn) begin
            if (HRESETn == 1'b0) begin
               latched_haddr[m] <= 32'h00000000;
            end else begin
               if (HREADY[m] & (HTRANS[m] != 2'b00)) begin
                   latched_haddr[m] <= HADDR[m];
                   //$display("bus cycle at address: %08x from master %d ", HADDR[m], m);
               end
            end
         end

         assign latched_haddr2[m] = (HREADY[m] & (HTRANS[m] != 2'b00)) ? HADDR[m] : latched_haddr[m];
*/
         // latch all address/control signals
         
         transparent_bus_latch #(32) tbl_haddr (
            .HCLOCK     (HCLOCK),
            .HRESETn    (HRESETn),
            .LATCH_OUT  (latched_haddr[m]),
            .HREADY     (HREADY[m]),
            .HTRANS_IS_NOT_0 (HTRANS[m] != 2'b00),
            .LATCH_IN   (HADDR[m])
         );
 
         transparent_bus_latch #(2) tbl_htrans (
            .HCLOCK     (HCLOCK),
            .HRESETn    (HRESETn),
            .LATCH_OUT  (latched_htrans[m]),
            .HREADY     (HREADY[m]),
            .HTRANS_IS_NOT_0 (HTRANS[m] != 2'b00),
            .LATCH_IN   (HTRANS[m])
         );
 
         transparent_latch tl_hwrite (
            .HCLOCK     (HCLOCK),
            .HRESETn    (HRESETn),
            .LATCH_OUT  (latched_hwrite[m]),
            .HREADY     (HREADY[m]),
            .HTRANS_IS_NOT_0 (HTRANS[m] != 2'b00),
            .LATCH_IN   (HWRITE[m])
         );
          
         transparent_latch tl_hlock (
            .HCLOCK     (HCLOCK),
            .HRESETn    (HRESETn),
            .LATCH_OUT  (latched_hlock[m]),
            .HREADY     (HREADY[m]),
            .HTRANS_IS_NOT_0 (HTRANS[m] != 2'b00),
            .LATCH_IN   (HLOCK[m])
         );
          
         transparent_bus_latch #(3) tbl_hburst (
            .HCLOCK     (HCLOCK),
            .HRESETn    (HRESETn),
            .LATCH_OUT  (latched_hburst[m]),
            .HREADY     (HREADY[m]),
            .HTRANS_IS_NOT_0 (HTRANS[m] != 2'b00),
            .LATCH_IN   (HBURST[m])
         );
 
         transparent_bus_latch #(3) tbl_hsize (
            .HCLOCK     (HCLOCK),
            .HRESETn    (HRESETn),
            .LATCH_OUT  (latched_hsize[m]),
            .HREADY     (HREADY[m]),
            .HTRANS_IS_NOT_0 (HTRANS[m] != 2'b00),
            .LATCH_IN   (HSIZE[m])
         );
 
         transparent_bus_latch #(4) tbl_hprot (
            .HCLOCK     (HCLOCK),
            .HRESETn    (HRESETn),
            .LATCH_OUT  (latched_hprot[m]),
            .HREADY     (HREADY[m]),
            .HTRANS_IS_NOT_0 (HTRANS[m] != 2'b00),
            .LATCH_IN   (HPROT[m])
         );
 
         latches_valid lv (
            .HCLOCK     (HCLOCK),
            .HRESETn    (HRESETn),
            .LATCHES_VALID  (latched_values_valid[m]),
            .HREADY     (HREADY[m]),
            .HTRANS_IS_NOT_0 (HTRANS[m] != 2'b00)
         );

         blgen gb (latched_byte_lanes[m], latched_hsize[m], latched_haddr[m]);

         always @(posedge HCLOCK) hready_delayed[m] <= HREADY[m];
         always @(posedge HCLOCK) hwrite_delayed[m] <= HWRITE[m];
         always @(posedge HCLOCK) htrans_delayed[m] <= HTRANS[m];

         transparent_bus_latch #(32) tbl_hwrite (
            .HCLOCK     (HCLOCK),
            .HRESETn    (HRESETn),
            .LATCH_OUT  (latched_hwdata[m]),
            .HREADY     (hready_delayed[m]),
            .HTRANS_IS_NOT_0 (hwrite_delayed[m] & (htrans_delayed[m] != 2'b00)),
            .LATCH_IN   (HWDATA[m])
         );

      end
   endgenerate

   generate
      for (s=0; s<slaves; s++) begin
         for (m=0; m<masters; m++) begin
            assign grant_array[s][m] = (grants[m] && (target[m] == s)); 
         end

         assign connected_address_phase[s] = oh_to_idx(grant_array[s]);

         assign connected_data_phase[s] = S_READY[s] ? connected_address_phase_delayed[s] : 4'b0000; 
         //assign connected_data_phase[s] = S_READY[s] ? connected_address_phase_delayed[s] : latched_connected_data_phase[s];
         
         always @(posedge HCLOCK) connected_address_phase_delayed[s] <= connected_address_phase[s];
         always @(posedge HCLOCK) if (S_READY[s]) latched_connected_data_phase[s] <= connected_address_phase_delayed[s];

         always @(posedge HCLOCK or negedge HRESETn) begin
            if (HRESETn == 1'b0) begin
               //connected_data_phase[s] <= 0;
               slave_busy_delayed[s] <= 0;
               slave_busy_data_phase[s] <= 0;
            end else begin
               // if (HREADY[connected_address_phase[s]]) begin
               slave_busy_delayed[s] <= slave_busy[s];
               if (S_READY[s]) begin
                   //connected_data_phase[s] <= connected_address_phase[s];
                   slave_busy_data_phase[s] <= slave_busy_delayed[s];
               end
            end
         end

      end
   endgenerate


   generate
      for (s=0; s<slaves; s++) begin

         assign slave_busy[s] = any_bit_set(grant_array[s]); 

         assign S_CHIP_SELECT[s]   = slave_busy[s] & latched_values_valid [connected_address_phase[s]]; 

         assign S_ADDRESS[s]       = slave_busy[s] ? latched_haddr      [connected_address_phase[s]] : 32'h00000000;
         assign S_BYTE_ENABLE[s]   = slave_busy[s] ? latched_byte_lanes [connected_address_phase[s]] :  4'h0;
         assign S_WRITE[s]         = slave_busy[s] ? latched_hwrite     [connected_address_phase[s]] :  1'b0;
         assign S_TRANS[s]         = slave_busy[s] ? latched_htrans     [connected_address_phase[s]] :  2'b00;

         assign S_WDATA[s]         = slave_busy_delayed[s] ? latched_hwdata[connected_data_phase[s]] : 32'h00000000;
      end
   endgenerate

endmodule
