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
// DESIGN UNIT:        ccs_ramifc_w_handshake_r
//
// DESCRIPTION:
//   Simply pass through the internal handshake signals for read_ram so the user can write
//   their own memory-access modues.
//   - Supports waiton so the core can be stalled for multi-cycle memory access 
//
// CHANGE LOG:
//   02/20/2019 - Creation
//
//
// --------------------------------------------------------------------------

module ccs_ramifc_w_handshake_r (clk_int, rstn_int, s_re_int, s_rrdy_int, s_raddr_int, s_din_int,
                                 clk,     rstn,     s_re,     s_rrdy,     s_raddr,     s_din);

   parameter   rscid    = 1;     // Resource ID
   parameter   depth    = 16;    // SCVerify needs depth for the transactor
   parameter   op_width = 1;     // dummy parameter for cwidth calculation
   parameter   width    = 16;    // Catapult data bus width (multiples of 8)
   parameter   addr_w   = 4;     // Catapult address bus width
   parameter   rst_ph   = 0;     // reset phase
   parameter   nopreload = 0; // SCVerify transactor needs this

   input                clk_int;
   input                rstn_int;
   output               s_rrdy_int;
   input                s_re_int;
   input [addr_w-1 : 0] s_raddr_int;
   output [width-1:0]   s_din_int;
   
   output               clk;
   output               rstn;
   input                s_rrdy;
   output               s_re;
   output [addr_w-1 : 0] s_raddr;
   input [width-1:0]     s_din;
   
   assign s_rrdy_int = s_rrdy;
   assign s_din_int  = s_din;
   assign s_re       = s_re_int;
   assign s_raddr    = s_raddr_int;
   assign clk        = clk_int;
   assign rstn       = rstn_int;
   
endmodule

