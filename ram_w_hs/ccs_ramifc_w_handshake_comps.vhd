--//////////////////////////////////////////////////////////////////////////////
-- Catapult Synthesis - Custom Interfaces
--
-- Copyright (c) 2019 Mentor Graphics Corp.
--       All Rights Reserved
-- 
-- This document contains information that is proprietary to Mentor Graphics
-- Corp. The original recipient of this document may duplicate this  
-- document in whole or in part for internal business purposes only, provided  
-- that this entire notice appears in all copies. In duplicating any part of  
-- this document, the recipient agrees to make every reasonable effort to  
-- prevent the unauthorized use and distribution of the proprietary information.
-- 
-- The design information contained in this file is intended to be an example
-- of the functionality which the end user may study in prepartion for creating
-- their own custom interfaces. This design does not present a complete
-- implementation of the named protocol or standard.
--
-- NO WARRANTY.
-- MENTOR GRAPHICS CORP. EXPRESSLY DISCLAIMS ALL WARRANTY
-- FOR THE SOFTWARE. TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE
-- LAW, THE SOFTWARE AND ANY RELATED DOCUMENTATION IS PROVIDED "AS IS"
-- AND WITH ALL FAULTS AND WITHOUT WARRANTIES OR CONDITIONS OF ANY
-- KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, WITHOUT LIMITATION, THE
-- IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
-- PURPOSE, OR NONINFRINGEMENT. THE ENTIRE RISK ARISING OUT OF USE OR
-- DISTRIBUTION OF THE SOFTWARE REMAINS WITH YOU.
-- 
--//////////////////////////////////////////////////////////////////////////////

-- --------------------------------------------------------------------------
-- LIBRARY: ccs_ramifc_w_handshake_lib
--
-- CONTENTS:
--    ccs_ramifc_w_handshake_r, ccs_ramifc_w_handshake_w
--      Ram w/handshake Interface Components
--    ccs_ram_w_handshake
--      Rtl memory module needed for the transactor
--
-- CHANGE LOG:
--
--  03/07/2019 - Initial implementation
--
-- --------------------------------------------------------------------------

-- --------------------------------------------------------------------------
-- PACKAGE:     ccs_ramifc_w_handshake_comps
--
-- DESCRIPTION:
--   Contains component declarations for all design units in this file.
--
-- CHANGE LOG:
--
--  03/07/2019 - Initial implementation
--
-- --------------------------------------------------------------------------

LIBRARY ieee;
   USE ieee.std_logic_1164.all;
   USE ieee.std_logic_arith.all;
   USE ieee.std_logic_unsigned.all;

PACKAGE ccs_ramifc_w_handshake_comps IS


  COMPONENT ccs_ramifc_w_handshake_r IS
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
      s_din   : in  std_logic_vector(width-1 downto 0)    -- Returned data
    );

  END COMPONENT;

  COMPONENT ccs_ramifc_w_handshake_w IS
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

      -- Catapult write channel control and data
      s_wrdy_int  : out std_logic;                            -- Memory ready for write request
      s_we_int    : in  std_logic;                            -- Write enable. Address & data valid
      s_waddr_int : in  std_logic_vector(addr_w-1 downto 0);  -- Write data address
      s_dout_int  : in  std_logic_vector(width-1 downto 0);   -- Data to write

      -- External write channel control and data to user memory
      clk     : out std_logic;       -- Rising edge clock
      rstn    : out std_logic;       -- Active LOW asynchronous reset
      s_wrdy  : in  std_logic;                            -- UserMem ready for write request
      s_we    : out std_logic;                            -- Write enable.  Address & data valid
      s_waddr : out std_logic_vector(addr_w-1 downto 0);  -- Write data address
      s_dout  : out std_logic_vector(width-1 downto 0)    -- Data to write
    );  
  END COMPONENT;

  COMPONENT ccs_ram_w_handshake IS
    GENERIC(
      ram_id          : integer                 := 1;     -- Resource ID
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
      clk    : in   std_logic;                    -- Rising edge clock
      rstn   : in   std_logic;                    -- Active LOW asynchronous reset
    
      -- Catapult read channel control and data
      s_re    : in  std_logic;                            -- Read enable.  Address valid
      s_rrdy  : out std_logic;                            -- Read data valid
      s_raddr : in  std_logic_vector(addr_w-1 downto 0);  -- Read data address
      s_din   : out std_logic_vector(width-1 downto 0);   -- Returned data
    
      -- Catapult write channel control and data
      s_we    : in  std_logic;                            -- Write enable. Address & data valid
      s_wrdy  : out std_logic;                            -- Memory ready for write request
      s_waddr : in  std_logic_vector(addr_w-1 downto 0);  -- Write data address
      s_dout  : in  std_logic_vector(width-1 downto 0)    -- Data to write
    );
END COMPONENT;

END ccs_ramifc_w_handshake_comps ;
