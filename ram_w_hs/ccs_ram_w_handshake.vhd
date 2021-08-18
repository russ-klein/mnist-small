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
-- DESIGN UNIT:        ccs_ram_w_handshake
--
-- DESCRIPTION:
--   This design unit models a generic slave memory using the internal catapult protol.
--   - Read and Write must be implemented for use in SCVerify
--   - Slave (to catapult) semantics.  The same protocol allows master memory implementation
--   - Supports burst, with latency for out-of-order addresses.
--   - Read and Write can happen concurrently
--   - Indeterminate if read/write to the same address at the same time
--
-- CHANGE LOG:
--   02/20/2019 - Creation
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

ENTITY  ccs_ram_w_handshake IS
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

END ccs_ram_w_handshake;

ARCHITECTURE rtl of ccs_ram_w_handshake IS
  constant latency : integer := 5;

  TYPE   read_state_t  IS (r_idle, r_read, r_stalling);
  TYPE   write_state_t  IS (w_idle, w_write, w_stalling);
  SIGNAL rState : read_state_t;
  SIGNAL wState : write_state_t;

  TYPE   mem_type IS ARRAY (depth-1 downto 0) of std_logic_vector(width-1 downto 0);
  SIGNAL memArray  : mem_type;

  SIGNAL    dataSent    : integer;
  SIGNAL    rStartAddr  : integer ;
  SIGNAL    rOutOfOrder : std_logic;
  SIGNAL    rStallCnt   : integer ;

  SIGNAL    dataRcvd    : integer ;
  SIGNAL    wStartAddr  : integer ;
  SIGNAL    wOutOfOrder : std_logic;
  SIGNAL    wStallCnt   : integer ;

  SIGNAL    int_rstn    : std_logic;
  
  SIGNAL    s_din_reg   : std_logic_vector(width-1 downto 0);
  SIGNAL    s_rrdy_reg  : std_logic;
  SIGNAL    s_wrdy_reg  : std_logic;

  function extendAddr(catAddr : std_logic_vector(addr_w -1 downto 0))
    return std_logic_vector is
  
    variable bigAddr : std_logic_vector(31 downto 0);
  
  begin
    bigAddr := (others => '0');
    bigAddr(addr_w -1 downto 0) := catAddr;
    return bigAddr;
  end function extendAddr;

BEGIN
  int_rstn <= rstn when (rst_ph = 0) else (not rstn);
  s_din <= s_din_reg ;

  rOutOfOrder <= '1' when (rState = r_stalling) or 
                 ((rState = r_read) and
                  (to_integer(unsigned(extendAddr(s_raddr)))+1 /= (rStartAddr + dataSent))and
                  (int_rstn = '1') and
                  (s_re = '1'))
                 else '0';
 
  s_rrdy <= s_rrdy_reg and not(rOutOfOrder);
  
  wOutOfOrder <= '1' when (wState = w_stalling) or
                 ((wState = w_write) and
                  ((to_integer(unsigned(extendAddr(s_waddr)))) /= (wStartAddr + dataRcvd)) and
                  (int_rstn = '1') and
                  (s_we = '1'))
                 else '0';
  s_wrdy <= s_wrdy_reg and not(wOutOfOrder);

    -- Read channel
  userRead :process(clk, int_rstn)
  begin
    if (int_rstn = '0') then
      dataSent <= 0;
      rStartAddr <= 0;
      s_rrdy_reg <= '0';
      rState <= r_idle;
      rStallCnt <= 0;
    elsif rising_edge(clk) then
      if (rState = r_idle) then
        if (s_re = '1') then
          s_din_reg <= memArray(to_integer(unsigned(s_raddr)));
          s_rrdy_reg <= '1';
          rStartAddr <= to_integer(unsigned(s_raddr));
          dataSent <= 1;
          rState <= r_read;
        else
          s_rrdy_reg <= '0';
        end if;
      elsif (rState = r_read) then
        -- "burst" in progress
        if (rOutOfOrder = '1') then 
          if (latency = 0) then
            rState <= r_idle;
          else
            rState <= r_stalling;
          end if;
          rStallCnt <= 0;
          s_rrdy_reg <= '0';
          dataSent <= 0;
          rStartAddr <= 0;
        else
          if (s_re = '1') then
            if (to_integer(unsigned(extendAddr(s_raddr)))+1 >= depth) then 
              s_rrdy_reg <= '0';
              dataSent <= 0;
              rStartAddr <= 0;               
              rState <= r_idle;
            else
              s_din_reg <= memArray(to_integer(unsigned(s_raddr))+1);
              s_rrdy_reg <= '1';
              dataSent <= dataSent+1;
            end if;
          else
            s_rrdy_reg <= '0';
            dataSent <= 0;
            rStartAddr <= 0;               
            rState <= r_idle;
          end if;
        end if;
      elsif (rState = r_stalling) then
        if (rStallCnt >= latency) then
          rStallCnt <= 0;
          rState <= r_idle;
        else
          rStallCnt <= rStallCnt + 1;
        end if;
      else
        report "Bad read state in ccs_ram_w_handshake"
          severity error;
      end if;
    end if;
  end  process;                            

      -- Read channel
  userWrite :process(clk, int_rstn)
  begin
    if (int_rstn = '0') then
      wState <= w_idle;
      wStartAddr <= 0;
      s_wrdy_reg <= '0';
      wStallCnt <= 0;
      dataRcvd  <= 0;
    elsif rising_edge(clk) then
      if (wState = w_idle) then
        if ((s_we = '1') and (s_wrdy_reg = '1')) then -- both have to happen to accept the data
          if (to_integer(unsigned(extendAddr(s_waddr))) >= depth) then
            report "Out of bounds array write in ccs_ram_w_handshake"
              severity error;
          else
            memArray(to_integer(unsigned(extendAddr(s_waddr)))) <= s_dout;
            wStartAddr <= to_integer(unsigned(s_waddr));
            dataRcvd <= 1;
            wState <= w_write;
          end if;
        else
          s_wrdy_reg <= '1';
        end if;
      elsif (wState = w_write) then
        if (wOutOfOrder = '1') then
          if (to_integer(unsigned(extendAddr(s_waddr))) >= depth) then
            report "Out of bounds array write in ccs_ram_w_handshake"
              severity error;
          else
            memArray(to_integer(unsigned(extendAddr(s_waddr)))) <= s_dout;
            wState <= w_stalling;
            wStallCnt <= 0;
            s_wrdy_reg <= '0';
            dataRcvd <= 0;
          end if;
        else
          if (s_we = '1') then
            if (to_integer(unsigned(extendAddr(s_waddr)))+1 >= depth) then
              memArray(to_integer(unsigned(extendAddr(s_waddr)))) <= s_dout;
              s_wrdy_reg <= '0';
              dataRcvd <= 0;
              wStartAddr <= 0;               
              wState <= w_idle;
            else
              memArray(to_integer(unsigned(extendAddr(s_waddr)))) <= s_dout;
              s_wrdy_reg <= '1';
              dataRcvd <= dataRcvd+1;
            end if;
          else 
            -- Start another burst
            s_wrdy_reg <= '1';
            dataRcvd <= 0;
            wStartAddr <= 0;
            wState <= w_idle;
          end if;
        end if;
      elsif (wState = w_stalling) then
        if (wStallCnt >= latency) then
          wStallCnt <= 0;
          wState <= w_idle;
          s_wrdy_reg <= '1';
        else
          wStallCnt <= wStallCnt + 1;
        end if;
      else
        report "Bad write state in ccs_ram_w_handshake"
          severity error;
      end if;
    end if;
  end process;
                                              
END rtl;
