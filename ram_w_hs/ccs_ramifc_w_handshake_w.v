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
// DESIGN UNIT:        ccs_ramifc_w_handshake_w
//
// DESCRIPTION:
//   Simply pass through the internal handshake signals for write_ram operator 
//   so the user can write their own memory-access modules.
//   - Supports waiton so the core can be stalled for multi-cycle memory access 
//
// CHANGE LOG:
//   02/20/2019 - Creation
//
//
// --------------------------------------------------------------------------

module ccs_ramifc_w_handshake_w (clk_int, rstn_int, s_we_int, s_wrdy_int, s_waddr_int, s_dout_int,
                                 clk,     rstn,     s_we,     s_wrdy,     s_waddr,     s_dout);

   parameter   rscid    = 1;     // Resource ID
   parameter   depth    = 16;    // SCVerify needs depth for the transactor
   parameter   op_width = 1;     // dummy parameter for cwidth calculation
   parameter   width    = 16;    // Catapult data bus width (multiples of 8)
   parameter   addr_w   = 4;     // Catapult address bus width
   parameter   rst_ph   = 0;     // reset phase
   parameter   nopreload = 0; // SCVerify transactor needs this

   input                clk_int;
   input                rstn_int;
   
   output               s_wrdy_int;
   input                s_we_int;
   input [addr_w-1 : 0] s_waddr_int;
   input [width-1:0]    s_dout_int;
   
   output                clk;
   output                rstn;
   input                 s_wrdy;
   output                s_we;
   output [addr_w-1 : 0] s_waddr;
   output [width-1:0]    s_dout;

   assign s_wrdy_int = s_wrdy;
   assign s_we       = s_we_int;
   assign s_waddr    = s_waddr_int;
   assign s_dout     = s_dout_int;
   assign clk        = clk_int;
   assign rstn       = rstn_int;
   
endmodule

