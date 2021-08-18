-------------------------------------------------------------------------------
-- Catapult Synthesis
--
-- Copyright (c) 2003-2019 Calypto Design Systems, Inc.
--       All Rights Reserved
--
-- This document contains information that is proprietary to Calypto Design
-- Systems, Inc. The original recipient of this document may duplicate this  
-- document in whole or in part for internal business purposes only, provided  
-- that this entire notice appears in all copies. In duplicating any part of  
-- this document, the recipient agrees to make every reasonable effort to  
-- prevent the unauthorized use and distribution of the proprietary information.
--//////////////////////////////////////////////////////////////////////////////
-- NO WARRANTY. CALYPTO DESIGN SYSTEMS INC. EXPRESSLY DISCLAIMS ALL WARRANTY 
-- FOR THE SOFTWARE. TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE 
-- LAW, THE SOFTWARE AND ANY RELATED DOCUMENTATION IS PROVIDED "AS IS"
-- AND WITH ALL FAULTS AND WITHOUT WARRANTIES OR CONDITIONS OF ANY 
-- KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, WITHOUT LIMITATION, THE 
-- IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
-- PURPOSE, OR NONINFRINGEMENT. THE ENTIRE RISK ARISING OUT OF USE OR 
-- DISTRIBUTION OF THE SOFTWARE REMAINS WITH YOU.
--
-- --------------------------------------------------------------------------
-- DESIGN UNIT:        ccs_ramifc_w_handshake_r
--
-- DESCRIPTION:
--   Simply pass through the internal handshake signals for read_ram so the user can write
--   their own memory-access modues.
--   - Supports waiton so the core can be stalled for multi-cycle memory access 
--
-- CHANGE LOG:
--   02/20/2019 - Creation
--
--
-- --------------------------------------------------------------------------

LIBRARY ieee;
  USE ieee.std_logic_1164.all;
  USE ieee.numeric_std.all;       

  USE std.textio.all;
  USE ieee.std_logic_textio.all;
  USE ieee.math_real.all;

LIBRARY ccs_ramifc_w_handshake_lib;
  USE  ccs_ramifc_w_handshake_lib.ccs_ramifc_w_handshake_comps.all;

ENTITY  ccs_ramifc_w_handshake_r IS
    GENERIC(
      rscid           : integer                 := 1;     -- Resource ID
      -- Array size generics
      depth           : integer                 := 16;     -- Memory array depth (#words)
      op_width        : integer range 1 to 1024 := 1;      -- dummy parameter for cwidth calculation
      width           : integer range 8 to 1024 := 32;     -- Memory array width
      addr_w          : integer range 1 to 64   := 4;      -- Catapult address bus width
      rst_ph          : integer range 0 to 1    := 0;      -- Reset phase - negative default
      nopreload       : integer range 0 to 1    := 0       -- SCVerify preload before eval(likely an input array)
    );
    PORT(
      -- Catapult interface
      clk_int    : in   std_logic;                    -- Rising edge clock
      rstn_int   : in   std_logic;                    -- Active LOW asynchronous reset

      -- Catapult read channel control and data
      s_rrdy_int  : out std_logic;                            -- Read data valid
      s_re_int    : in  std_logic;                            -- Read enable.  Address valid
      s_raddr_int : in  std_logic_vector(addr_w-1 downto 0);  -- Read data address
      s_din_int   : out std_logic_vector(width-1 downto 0);   -- Returned data

      -- External read channel control and data to user memory
      clk     : out std_logic;       -- Rising edge clock
      rstn    : out std_logic;       -- Active LOW asynchronous reset
      s_rrdy  : in  std_logic;       -- Read data valid
      s_re    : out std_logic;       -- Read enable.  Address valid
      s_raddr : out std_logic_vector(addr_w-1 downto 0);  -- Read data address
      s_din   : in  std_logic_vector(width-1 downto 0)   -- Returned data
    );

END ccs_ramifc_w_handshake_r;

ARCHITECTURE rtl of ccs_ramifc_w_handshake_r IS

BEGIN
  
  s_rrdy_int <= s_rrdy;
  s_din_int  <= s_din;
  s_re       <= s_re_int;
  s_raddr    <= s_raddr_int;
  clk        <= clk_int;
  rstn       <= rstn_int;
  
END rtl;
