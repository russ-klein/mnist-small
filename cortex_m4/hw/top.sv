

module top (
   input           HCLK, 
   input           PORESETn,

   output [31:0]   TB_ADDRESS,
   output [3:0]    TB_BL,
   output          TB_WE,
   output          TB_CE,
   output [31:0]   TB_WDATA,
   input  [31:0]   TB_RDATA,
   output [1:0]    TB_TRANS,
   input  [1:0]    TB_RESP,
   input           TB_READY 

);

`ifdef TB_MEM
`define top_mem_bits 5
`else 
`define top_mem_bits 18
`endif

`ifdef MASTER
`define masters 4
`define slaves  6
`else
`define masters 3
`define slaves  6
`endif

   // Clocks and resets

   wire          SYSRESETn = PORESETn;      // System reset
   wire          HRESETn = PORESETn;        // AHB reset
   wire          HCLOCK = HCLK;             // AHB bus clock
   wire          FCLK = HCLK;               // Free-running clock
   wire          STCLK = 1'b0;              // System Tick enable
   wire          DAPCLK = HCLK;             // DAP clock
   wire          DAPCLKEN = 1'b1;           // DAP clock enable
   wire          DAPRESETn = PORESETn;      // DAP reset


   // Signals for DAP JTAG controller

  wire           SRST;
  wire           nTRST;                     // JTAG TAP Reset
  wire           SWCLKTCK;                  // SW/JTAG Clock
  wire           SWDITMS;                   // SW Debug Data In / JTAG Test Mode Select
  wire           TDI;                       // JTAG TAP Data In / Alternative input function
  wire           nCDBGPWRDN = 1'b1;         // Debug infrastructure power-down control
  wire           CDBGPWRUPACK = 1'b1;       // Debug Power Domain up acknowledge
  wire           CSYSPWRUPACK = 1'b1;       // System Power Domain up acknowledge
  wire           CDBGRSTACK = 1'b0;         // Debug reset acknowledge to reset controller

  wire           SWDO;                      // SW Data Out
  wire           SWDOEN;                    // SW Data Out Enable
  wire           TDO;                       // JTAG TAP Data Out
  wire           nTDOEN;                    // TDO enable
  wire           CDBGPWRUPREQ;              // Debug Power Domain up request
  wire           CSYSPWRUPREQ;              // System Power Domain up request
  wire           CDBGRSTREQ;                // Debug reset request to reset controller
  wire           JTAGNSW;                   // JTAG/not Serial Wire Mode
  wire           JTAGTOP;                   // JTAG-DP TAP controller in one of four top states
                                            // (TLR, RTI, Sel-DR, Sel-IR)

   // ICode bus

   wire          HREADYI;                   // ICode ready
   wire [31:0]   HRDATAI;                   // ICode read data
   wire [1:0]    HRESPI;                    // ICode transfer response
   wire          IFLUSH = 1'b0;             // ICode buffer flush

   // DCode bus

   wire          HREADYD;                   // DCode ready
   wire [31:0]   HRDATAD;                   // DCode read data
   wire [1:0]    HRESPD;                    // DCode transfer response
   wire          EXRESPD;                   // DCode exclusive response

   // System bus

   wire          HREADYS;                   // System ready
   wire [31:0]   HRDATAS;                   // System read data
   wire [1:0]    HRESPS;                    // System transfer response
   wire          EXRESPS;                   // System exclusive response

   // External PPB

   wire [31:0]   PRDATA;                    // PPB read data
   wire          PREADY = 1'b1;             // PPB ready
   wire          PSLVERR = 1'b0;            // PPB slave error

   // DAP

   wire          DAPEN = 1'b1;              // DAP enable
   wire          DAPSEL;                    // DAP select
   wire          DAPENABLE;                 // DAP enable
   wire          DAPWRITE;                  // DAP write
   wire          DAPABORT;                  // DAP abort
   wire [31:0]   DAPADDR;                   // DAP address
   wire [31:0]   DAPWDATA;                  // DAP write data
   wire          FIXMASTERTYPE = 1'b0;      // Force HMASTER setting for AHB-AP accesses

   // Interrupts interface

   wire [239:0]  INTISR = {240{1'b0}};      // Interrupts
   wire          INTNMI = 1'b0;             // Non-maskable interrupt

   // WIC interface

   wire          WICDSREQn = 1'b1;          // Use WIC Request

   // ATB interface

   wire          ATREADY = 1'b0;            // ATB Ready

   // Events

   wire          RXEV = 1'b0;               // Wait for event wire

   // ETM interface

   wire          ETMPWRUP = 1'b0;           // ETM enabled
   wire          ETMFIFOFULL = 1'b0;        // ETM FIFO is full

   // Debug

   wire          EDBGRQ;                    // External debug request
   wire          DBGRESTART = 1'b0;         // External debug restart request

   // TPIU interface

   wire          TPIUACTV = 1'b0;           // TPIU has data
   wire          TPIUBAUD = 1'b0;           // Unsynchronised baud indicator from TPIU

   // Sleep interface

   wire          SLEEPHOLDREQn = 1'b1;      // Hold core in sleep mode

   // Auxiliary fault status

   wire [31:0]   AUXFAULT = 32'h0;          // Auxillary FSR pulse wires

   // Test interface

   wire          SE = 1'b0;                 // Scan enable
   wire          RSTBYPASS = 1'b0;          // Reset Bypass
   wire          CGBYPASS = 1'b0;           // Architectural clock gate bypass

   // Configuration inputs

   wire [25:0]   STCALIB = 26'h0;           // System Tick calibration
   wire          BIGEND = 1'b0;             // Static endianess select
   wire          DNOTITRANS = 1'b0;         // D/ICODE arbitration
   wire          STKALIGNINIT = 1'b1;       // STKALIGN reset value

   // Miscellaneous

   wire [5:0]    PPBLOCK = 1'b0;            // PPB Lock
   wire [9:0]    VECTADDR = 10'h0;          // Vector address overwrite value
   wire          VECTADDREN = 1'b0;         // Enable IntAddr -> VECTADDR overwrite

   // Global timestamp interface

   wire [47:0]   TSVALUEB = 48'h0;          // Binary coded timestamp count
   wire          TSCLKCHANGE = 1'b0;        // Pulse when TS clock ratio changes

   // Logic disable

   wire          MPUDISABLE = 1'b0;         // Disable the MPU (act as default)
   wire          DBGEN = 1'b1;              // Enable debug

   // ICode bus

   wire [31:0]   HADDRI;                    // ICode address
   wire [1:0]    HTRANSI;                   // ICode transfer type
   wire [2:0]    HSIZEI;                    // ICode transfer size
   wire [2:0]    HBURSTI;                   // ICode burst length
   wire [3:0]    HPROTI;                    // ICode protection
   wire [1:0]    MEMATTRI;                  // ICode memory attributes
   wire [3:0]    BRCHSTAT;                  // Branch status hint

   // DCode bus

   wire [31:0]   HADDRD;                    // DCode address
   wire [1:0]    HTRANSD;                   // DCode transfer type
   wire [1:0]    HMASTERD;                  // DCode master
   wire [2:0]    HSIZED;                    // DCode transfer size
   wire [2:0]    HBURSTD;                   // DCode burst length
   wire [3:0]    HPROTD;                    // DCode protection
   wire [1:0]    MEMATTRD;                  // DCode memory attributes
   wire          EXREQD;                    // DCode exclusive request
   wire          HWRITED;                   // DCode write not read
   wire [31:0]   HWDATAD;                   // DCode write data
 
   // System bus

   wire [31:0]   HADDRS;                    // System address
   wire [1:0]    HTRANSS;                   // System transfer type
   wire [1:0]    HMASTERS;                  // System master
   wire          HWRITES;                   // System write not read
   wire [2:0]    HSIZES;                    // System transfer size
   wire          HMASTLOCKS;                // System lock
   wire [31:0]   HWDATAS;                   // System write data
   wire [2:0]    HBURSTS;                   // System burst length
   wire [3:0]    HPROTS;                    // System protection
   wire [1:0]    MEMATTRS;                  // System memory attributes
   wire          EXREQS;                    // System exclusive request

   // External PPB

   wire          PSEL;                      // PPB External select
   wire          PADDR31;                   // PPB External address, bit 31
   wire [19:2]   PADDR;                     // PPB External address
   wire          PENABLE;                   // PPB External enable
   wire          PWRITE;                    // PPB External write not read
   wire [31:0]   PWDATA;                    // PPB External write data

   // DAP read data and DAPREADY multiplexer Signals
   wire          DAPREADY0 = 1'b1;          // DAP Ready from AHB-AP
   wire          DAPSLVERR0 = 1'b0;         // DAP slave error from AHB-AP
   wire [31:0]   DAPRDATA0 = 32'h0;         // DAP Read data from AHB-AP
   wire [31:0]   DAPRDATA1 = 32'h0;         // DAP Read data from APB-AP
   wire [31:0]   DAPRDATA2 = 32'h0;         // DAP Read data from JTAG-AP
   wire [31:0]   DAPRDATACM3;               // DAP Read data from CM3
   wire          DAPREADY1 = 1'b1;          // DAP Ready from APB-AP
   wire          DAPREADY2 = 1'b1;          // DAP Ready from JTAG-AP
   wire          DAPREADYCM3;               // DAP Ready from CM3
   wire          DAPSLVERR1 = 1'b0;         // DAP slave error from APB-AP
   wire          DAPSLVERR2 = 1'b0;         // DAP slave error from JTAG-AP
   wire          DAPSLVERRCM3;              // DAP slave error from CM3
   wire          DAPSELCM3;                 // DAP slave select for CM3
   wire          DAPSEL2;                   // DAP slave 2 select (JTAG-AP)
   wire          DAPSEL1;                   // DAP slave 1 select (APB-AP)
   wire          DAPSEL0;                   // DAP slave 0 select (AHB-AP)
   wire          DAPSELDEF;                 // DAP select for default AP (all others) 
   
   // DAP
   wire          DAPSLVERR;                 // DAP slave error
   wire [31:0]   DAPRDATA;                  // DAP read data

   // WIC interface

   wire          WICDSACKn;                 // WIC mode acknowledge
   wire          WICLOAD;                   // Load wake-up sensitiviy list into WIC
   wire          WICCLEAR;                  // Clear WIC sensitivity list
   wire [239:0]  WICMASKISR;                // WIC IRQ sensitivity list
   wire          WICMASKMON;                // WIC DBG MON sensitivity
   wire          WICMASKNMI;                // WIC NMI sensitivity
   wire          WICMASKRXEV;               // WIC RXEV sensitivity

   // ATB interface

   wire          ATVALID;                   // ATB Valid
   wire          AFREADY;                   // ATB Flush
   wire [7:0]    ATDATA;                    // ATB Data

   // Events

   wire          TXEV;                      // Event transmitted
   wire          SYSRESETREQ;               // System reset request

   // ETM interface

   wire [3:0]    ETMTRIGGER;                // DWT generated trigger
   wire [3:0]    ETMTRIGINOTD;              // ETM trigger on I match
   wire          ETMIVALID;                 // Instruction valid
   wire          ETMISTALL;                 // Instruction stall
   wire          ETMDVALID;                 // Data valid
   wire          ETMFOLD;                   // Instruction is folded
   wire          ETMCANCEL;                 // Instruction cancelled
   wire [31:1]   ETMIA;                     // PC to ETM
   wire          ETMICCFAIL;                // Instruction failed condition code
   wire          ETMIBRANCH;                // Instruction is Branch
   wire          ETMIINDBR;                 // Instruction is indirect branch
   wire          ETMISB;                    // Instruction is ISB
   wire [2:0]    ETMINTSTAT;                // ETM interrupt status
   wire [8:0]    ETMINTNUM;                 // ETM interrupt number
   wire          ETMFLUSH;                  // Pipe flush hint
   wire          ETMFINDBR;                 // Flush indirect branch
   wire          DSYNC;                     // Sync packet generation trigger

   // Debug

   wire          DBGRESTARTED;              // External Debug Restart Ready
 
   // HTM interface

   wire [31:0]   HTMDHADDR;                 // HTM data HADDR
   wire [1:0]    HTMDHTRANS;                // HTM data HTRANS
   wire [2:0]    HTMDHSIZE;                 // HTM data HSIZE
   wire [2:0]    HTMDHBURST;                // HTM data HBURST
   wire [3:0]    HTMDHPROT;                 // HTM data HPROT
   wire [31:0]   HTMDHWDATA;                // HTM data HWDATA
   wire          HTMDHWRITE;                // HTM data HWRITE
   wire [31:0]   HTMDHRDATA;                // HTM data HRDATA
   wire          HTMDHREADY;                // HTM data HREADY
   wire [1:0]    HTMDHRESP;                 // HTM data HRESP

   // ITM interface

   wire [6:0]    ATIDITM;                   // ITM ID for TPIU

   // Sleep interface
   wire          SLEEPHOLDACKn;             // Core is held in sleepmode
   wire          SLEEPING;                  // Core is Sleeping
   wire          SLEEPDEEP;                 // System can enter Deep sleep

   // Core status

   wire          HALTED;                    // Core is Stopped
   wire          LOCKUP;                    // Core is Locked Up
   wire [7:0]    CURRPRI;                   // Current INT priority
   wire          TRCENA;                    // Trace Enable

   // Extended visibility signals

   wire [148:0]  INTERNALSTATE;             // Exported internal state

   wire          FPIXC;                     // FPSCR IXC (cumulative inexact exception)
   wire          FPOFC;                     // FPSCR IXC (cumulative overflow exception)
   wire          FPUFC;                     // FPSCR IXC (cumulative underflow exception)
   wire          FPIOC;                     // FPSCR IXC (cumulative invalid op exception)
   wire          FPDZC;                     // FPSCR IXC (cumulative divide-by-zero exception)
   wire          FPIDC;                     // FPSCR IXC (cumulative input denormal exception)
   wire          FPUDISABLE = 0;            // Disable the FPU

   // AHB signals - Master Side

   wire          HCLKEN[`masters-1:0];
   wire [31:0]   HADDR[`masters-1:0];
   wire [1:0]    HTRANS[`masters-1:0];
   wire [2:0]    HBURST[`masters-1:0];
   wire          HWRITE[`masters-1:0];
   wire [2:0]    HSIZE[`masters-1:0];
   wire [3:0]    HPROT[`masters-1:0];
   wire          HGRANT[`masters-1:0];
   wire          HREADY[`masters-1:0];
   wire [1:0]    HRESP[`masters-1:0];
   wire [31:0]   HWDATA[`masters-1:0];
   wire [31:0]   HRDATA[`masters-1:0];
   wire          HBUSREQ[`masters-1:0];
   wire          HLOCK[`masters-1:0];
   wire [1:0]    HMEMATTR[`masters-1:0];
   wire [1:0]    HMASTER[`masters-1:0];

   // Slave Interfaces

   wire [31:0]   S_ADDRESS[`slaves-1:0];
   wire [3:0]    S_BL[`slaves-1:0];
   wire          S_WE[`slaves-1:0];
   wire          S_CE[`slaves-1:0];
   wire [31:0]   S_WDATA[`slaves-1:0];
   wire [31:0]   S_RDATA[`slaves-1:0];

   wire [1:0]    S_TRANS[`slaves-1:0];
   wire [1:0]    S_RESP[`slaves-1:0];
   wire          S_READY[`slaves-1:0];

   wire [7:0]    char_in;
   wire [7:0]    char_out;
   wire          strobe_in;
   wire          strobe_out;

   timer timer0 (
        .clock       (HCLK),
        .resetn      (HRESETn),
        .address     (S_ADDRESS[4][31:2]),
        .trans       (S_TRANS[4]),
        .bl          (S_BL[4]),
        .we          (S_WE[4]),
        .ce          (S_CE[4]),
        .write_data  (S_WDATA[4]),
        .read_data   (S_RDATA[4]),
        .resp        (S_RESP[4]),
        .ready       (S_READY[4])
   );


`ifdef SIMULATION
   char_in in_port (
        .clk         (HCLK),
        .resetn      (HRESETn),
        .char        (char_in),
        .strobe      (strobe_in)
   );

   char_out out_port (
        .clk         (HCLK),
        .resetn      (HRESETn),
        .char        (char_out),
        .strobe      (strobe_out)
   );
`else
   assign char_in = 8'h00;
   assign strobe_in = 1'b0;
`endif
`ifdef MASTER
   cat_accel go_fast (
        .clock       (HCLK),
        .resetn      (HRESETn),
        .address     (S_ADDRESS[3][31:2]),
        .trans       (S_TRANS[3]),
        .bl          (S_BL[3]),
        .we          (S_WE[3]),
        .ce          (S_CE[3]),
        .write_data  (S_WDATA[3]),
        .read_data   (S_RDATA[3]),
        .resp        (S_RESP[3]),
        .ready       (S_READY[3]),

        .haddr       (HADDR[3]),
        .htrans      (HTRANS[3]),
        .hburst      (HBURST[3]),
        .hwrite      (HWRITE[3]),
        .hsize       (HSIZE[3]),
        .hprot       (HPROT[3]),
        .hready      (HREADY[3]),
        .hresp       (HRESP[3]),
        .hwdata      (HWDATA[3]),
        .hrdata      (HRDATA[3]),
        .hlock       (HLOCK[3])
   );
`else
   cat_accel go_fast (
        .clock       (HCLK),
        .resetn      (HRESETn),
        .address     (S_ADDRESS[3][31:2]),
        .trans       (S_TRANS[3]),
        .bl          (S_BL[3]),
        .we          (S_WE[3]),
        .ce          (S_CE[3]),
        .write_data  (S_WDATA[3]),
        .read_data   (S_RDATA[3]),
        .resp        (S_RESP[3]),
        .ready       (S_READY[3])
   );
`endif
   uart #(.wait_states(1)) terminal (
        .CLOCK       (HCLK),
        .RESETn      (HRESETn),
        .ADDRESS     (S_ADDRESS[2][31:2]),
        .TRANS       (S_TRANS[2]),
        .BL          (S_BL[2]),
        .WE          (S_WE[2]),
        .CE          (S_CE[2]),
        .WDATA       (S_WDATA[2]),
        .RDATA       (S_RDATA[2]),
        .RESP        (S_RESP[2]),
        .READY       (S_READY[2]),

        .char_in_from_tbx   (char_in),
        .input_strobe       (strobe_in),
        .char_out_to_tbx    (char_out),
        .output_strobe      (strobe_out)
   );

`ifdef TB_MEM
   assign S_READY[1] = 1'b0;
   assign S_RDATA[1] = 32'h00000000;
   assign S_RESP[1]  = 2'b00;
`else
   sram #(
        .file_no     (0),
        .addr_bits   (20),
        .wait_states (0)
     ) mem1 (
        .clock       (HCLK),
        .resetn      (HRESETn),
        .address     (S_ADDRESS[1]),
        .trans       (S_TRANS[1]),
        .bl          (S_BL[1]),
        .we          (S_WE[1]),
        .ce          (S_CE[1]),
        .write_data  (S_WDATA[1]),
        .read_data   (S_RDATA[1]),
        .resp        (S_RESP[1]),
        .ready       (S_READY[1])
   );
`endif

   sram #(
        .file_no     (0),
        .addr_bits   (18),
        .wait_states (0)
     ) mem0 (
        .clock       (HCLK),
        .resetn      (HRESETn),
        .address     (S_ADDRESS[0]),
        .trans       (S_TRANS[0]),
        .bl          (S_BL[0]),
        .we          (S_WE[0]),
        .ce          (S_CE[0]),
        .write_data  (S_WDATA[0]),
        .read_data   (S_RDATA[0]),
        .resp        (S_RESP[0]),
        .ready       (S_READY[0])
   );

   ahb_switch #(
        .masters(`masters),
        .slaves (`slaves)
   ) sw0 (
        .HCLOCK      (HCLK),
        .HRESETn     (HRESETn),

        .HADDR       (HADDR),
        .HTRANS      (HTRANS),
        .HBURST      (HBURST),
        .HWRITE      (HWRITE),
        .HSIZE       (HSIZE),
        .HPROT       (HPROT),
        .HREADY      (HREADY),
        .HRESP       (HRESP),
        .HWDATA      (HWDATA),
        .HRDATA      (HRDATA),
        .HLOCK       (HLOCK),

        .S_ADDRESS   (S_ADDRESS),
        .S_CHIP_SELECT (S_CE),
        .S_BYTE_ENABLE (S_BL),
        .S_WRITE     (S_WE),
        .S_WDATA     (S_WDATA),
        .S_TRANS     (S_TRANS),
        .S_RDATA     (S_RDATA),
        .S_RESP      (S_RESP),
        .S_READY     (S_READY)
   );

   // connect to test bench memory

   assign TB_ADDRESS    = S_ADDRESS[5];
   assign TB_BL         = S_BL[5];
   assign TB_WE         = S_WE[5];
   assign TB_CE         = S_CE[5];
   assign TB_WDATA      = S_WDATA[5];
   assign S_RDATA[5]    = TB_RDATA;
   assign TB_TRANS      = S_TRANS[5];
   assign S_RESP[5]     = TB_RESP;
   assign S_READY[5]    = TB_READY;

   // tie-off unused signals

   assign HWDATA[0] = 32'h00000000;    
   assign HWRITE[0] = 1'b0;

   assign HLOCK[0] = 1'b0;
   assign HLOCK[1] = 1'b0;

   CORTEXM4 cpu(
        // Inputs

        .PORESETn(PORESETn), 
        .SYSRESETn(SYSRESETn), 

        .FCLK(FCLK), 
        .HCLK(HCLK), 
        .STCLK(STCLK), 

        .STCALIB(STCALIB), 
        .BIGEND(BIGEND), 
        .INTISR(INTISR), 
        .INTNMI(INTNMI), 
        .RXEV(RXEV),
        .EDBGRQ(EDBGRQ), 

        // Instruction AMBA interface

        .HADDRI(HADDR[0]), 
        .HTRANSI(HTRANS[0]), 
        .HBURSTI(HBURST[0]), 
        .HSIZEI(HSIZE[0]), 
        .MEMATTRI(HMEMATTR[0]),
        .HPROTI(HPROT[0]), 
        .HREADYI(HREADY[0]), 
        .HRESPI(HRESP[0]), 
        .HRDATAI(HRDATA[0]), 
        .BRCHSTAT(BRCHSTAT),  // branch statistics

        // Data AMBA interface 

        .HADDRD(HADDR[1]), 
        .HTRANSD(HTRANS[1]), 
        .HBURSTD(HBURST[1]), 
        .HWRITED(HWRITE[1]),
        .HSIZED(HSIZE[1]), 
        .MEMATTRD(HMEMATTR[1]), 
        .HPROTD(HPROT[1]), 
        .HREADYD(HREADY[1]), 
        .HRESPD(HRESP[1]), 
        .HWDATAD(HWDATA[1]), 
        .HRDATAD(HRDATA[1]), 
        .HMASTERD(HMASTER[1]), 
        .EXRESPD(EXRESPD), 

        // System Bus 

        .HADDRS(HADDR[2]), 
        .HTRANSS(HTRANS[2]), 
        .HBURSTS(HBURST[2]), 
        .HWRITES(HWRITE[2]), 
        .HSIZES(HSIZE[2]), 
        .MEMATTRS(HMEMATTR[2]), 
        .HPROTS(HPROT[2]), 
        .HREADYS(HREADY[2]), 
        .HRESPS(HRESP[2]), 
        .HMASTLOCKS(HLOCK[2]), 
        .HWDATAS(HWDATA[2]),
        .HRDATAS(HRDATA[2]), 
        .HMASTERS(HMASTER[2]), 
        .EXRESPS(EXRESPS), 

        .IFLUSH(IFLUSH),   
        .PRDATA(PRDATA), 
        .PREADY(PREADY), 
        .PSLVERR(PSLVERR), 
        .DAPEN(DAPEN), 
        .DAPCLKEN(DAPCLKEN),
        .DAPCLK(DAPCLK), 
        .DAPRESETn(DAPRESETn), 
        .DAPSEL(DAPSELCM3), 
        .DAPENABLE(DAPENABLE), 
        .DAPWRITE(DAPWRITE), 
        .DAPABORT(DAPABORT), 
        .DAPADDR(DAPADDR), 
        .DAPWDATA(DAPWDATA),
        .SE(SE), 
        .ATREADY(ATREADY), 
        .PPBLOCK(PPBLOCK), 
        .VECTADDR(VECTADDR), 
        .VECTADDREN(VECTADDREN), 
        .ETMPWRUP(ETMPWRUP), 
        .ETMFIFOFULL(ETMFIFOFULL), 
        .TPIUACTV(TPIUACTV),
        .TPIUBAUD(TPIUBAUD), 
        .AUXFAULT(AUXFAULT), 
        .DNOTITRANS(DNOTITRANS), 
        .DBGRESTART(DBGRESTART), 
        .SLEEPHOLDREQn(SLEEPHOLDREQn), 
        .RSTBYPASS(RSTBYPASS),
        .CGBYPASS(CGBYPASS), 
        .STKALIGNINIT(STKALIGNINIT), 
        .WICDSREQn(WICDSREQn), 
        .FIXMASTERTYPE(FIXMASTERTYPE), 
        .TSVALUEB(TSVALUEB), 
        .TSCLKCHANGE(TSCLKCHANGE),
        .MPUDISABLE(MPUDISABLE), 
	.FPUDISABLE(FPUDISABLE),
        .DBGEN(DBGEN),

        // Outputs

        .SYSRESETREQ(SYSRESETREQ), 
        .TXEV(TXEV), 
        .EXREQD(EXREQD), 
        .EXREQS(EXREQS), 
        .PADDR(PADDR), 
        .PADDR31(PADDR31), 
        .PSEL(PSEL), 
        .PENABLE(PENABLE), 
        .PWRITE(PWRITE),
        .PWDATA(PWDATA), 
        .DAPREADY(DAPREADYCM3), 
        .DAPSLVERR(DAPSLVERRCM3), 
        .DAPRDATA(DAPRDATACM3), 
        .ATVALID(ATVALID), 
        .AFREADY(AFREADY), 
        .ATDATA(ATDATA), 
        .ETMTRIGGER(ETMTRIGGER),
        .ETMTRIGINOTD(ETMTRIGINOTD), 
        .ETMIVALID(ETMIVALID), 
        .ETMISTALL(ETMISTALL), 
        .ETMDVALID(ETMDVALID), 
        .ETMFOLD(ETMFOLD), 
        .ETMCANCEL(ETMCANCEL), 
        .ETMIA(ETMIA),
        .ETMICCFAIL(ETMICCFAIL), 
        .ETMIBRANCH(ETMIBRANCH), 
        .ETMIINDBR(ETMIINDBR), 
        .ETMISB(ETMISB), 
        .ETMINTSTAT(ETMINTSTAT), 
        .ETMINTNUM(ETMINTNUM), 
        .ETMFLUSH(ETMFLUSH),
        .ETMFINDBR(ETMFINDBR), 
        .DSYNC(DSYNC), 
        .HTMDHADDR(HTMDHADDR), 
        .HTMDHTRANS(HTMDHTRANS), 
        .HTMDHSIZE(HTMDHSIZE), 
        .HTMDHBURST(HTMDHBURST), 
        .HTMDHPROT(HTMDHPROT),
        .HTMDHWDATA(HTMDHWDATA), 
        .HTMDHWRITE(HTMDHWRITE), 
        .HTMDHRDATA(HTMDHRDATA), 
        .HTMDHREADY(HTMDHREADY), 
        .HTMDHRESP(HTMDHRESP), 
        .ATIDITM(ATIDITM), 
        .HALTED(HALTED), 
        .DBGRESTARTED(DBGRESTARTED), 
        .LOCKUP(LOCKUP), 
        .SLEEPING(SLEEPING), 
        .SLEEPDEEP(SLEEPDEEP), 
        .SLEEPHOLDACKn(SLEEPHOLDACKn), 
        .CURRPRI(CURRPRI),
        .TRCENA(TRCENA), 
        .WICDSACKn(WICDSACKn), 
        .WICLOAD(WICLOAD), 
        .WICCLEAR(WICCLEAR), 
        .WICMASKISR(WICMASKISR), 
        .WICMASKMON(WICMASKMON),
        .WICMASKNMI(WICMASKNMI), 
        .WICMASKRXEV(WICMASKRXEV),
        .FPIXC(FPIXC),
        .FPOFC(FPOFC),
        .FPUFC(FPUFC),
        .FPIOC(FPIOC),
        .FPDZC(FPDZC),
        .FPIDC(FPIDC)
  );    
/*
  reg hreadyi_delayed;
  reg hreadyd_delayed;
  reg hreadys_delayed;
  reg [1:0] htransi_delayed;
  reg [1:0] htransd_delayed;
  reg [1:0] htranss_delayed;
  reg hwritei_delayed;
  reg hwrited_delayed;
  reg hwrites_delayed;
  reg [31:0] haddri_delayed;
  reg [31:0] haddrd_delayed;
  reg [31:0] haddrs_delayed;

  always @(posedge HCLK) begin
     hreadyi_delayed <= HREADY[0];
     hreadyd_delayed <= HREADY[1];
     hreadys_delayed <= HREADY[2];
     htransi_delayed <= HTRANS[0];
     htransd_delayed <= HTRANS[1];
     htranss_delayed <= HTRANS[2];
     hwritei_delayed <= HWRITE[0];
     hwrited_delayed <= HWRITE[1];
     hwrites_delayed <= HWRITE[2];
     haddri_delayed  <= HADDR[0];
     haddrd_delayed  <= HADDR[1];
     haddrs_delayed  <= HADDR[2];
  end

  always @(posedge HCLK) begin
     if (hreadyi_delayed & (htransi_delayed != 2'b00) &  hwritei_delayed) $display("(%10t) >>> CPU:I: write: %08x @ %08x ", $time, HWDATA[0], haddri_delayed);
     if (hreadyi_delayed & (htransi_delayed != 2'b00) & !hwritei_delayed) $display("(%10t) >>> CPU:I: read:  %08x @ %08x ", $time, HRDATA[0], haddri_delayed);
     if (hreadyd_delayed & (htransd_delayed != 2'b00) &  hwrited_delayed) $display("(%10t) >>> CPU:D: write: %08x @ %08x ", $time, HWDATA[1], haddrd_delayed);
     if (hreadyd_delayed & (htransd_delayed != 2'b00) & !hwrited_delayed) $display("(%10t) >>> CPU:D: read:  %08x @ %08x ", $time, HRDATA[1], haddrd_delayed);
     if (hreadys_delayed & (htranss_delayed != 2'b00) &  hwrites_delayed) $display("(%10t) >>> CPU:S: write: %08x @ %08x ", $time, HWDATA[2], haddrs_delayed);
     if (hreadys_delayed & (htranss_delayed != 2'b00) & !hwrites_delayed) $display("(%10t) >>> CPU:S: read:  %08x @ %08x ", $time, HRDATA[2], haddrs_delayed);
   end
*/
endmodule
