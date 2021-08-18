#ifndef __INCLUDED_ccs_ram_w_handshake_trans_rsc_H__
#define __INCLUDED_ccs_ram_w_handshake_trans_rsc_H__
#include <mc_transactors.h>

//#define DBG_CB 1

#include <mc_simulator_extensions.h>

#define USER_MEMORY_MODULE "mymem"


template < 
  int depth
  ,int width
  ,int addr_w
  ,int rst_ph
>
class ccs_UserMemory_wrapper: public mc_foreign_module
{
public:
  // Interface Ports
  sc_in<bool>            clk;
  sc_in<sc_logic>        rstn;

  sc_in<sc_logic>        s_re;
  sc_out<sc_logic>       s_rrdy;
  sc_in< sc_lv<addr_w> > s_raddr;
  sc_out< sc_lv<width> > s_din;

  sc_in<sc_logic>        s_we;
  sc_out<sc_logic>       s_wrdy;
  sc_in< sc_lv<addr_w> > s_waddr;
  sc_in< sc_lv<width> >  s_dout;

public:
 ccs_UserMemory_wrapper(const sc_module_name& nm, const char *hdl_name)
   :
  mc_foreign_module(nm, hdl_name), 
    clk("clk"), 
    rstn("rstn"), 
    s_re("s_re"),
    s_rrdy("s_rrdy"),
    s_raddr("s_raddr"),
    s_din("s_din"),
    s_we("s_we"),
    s_wrdy("s_wrdy"),
    s_waddr("s_waddr"),
    s_dout("s_dout")
      {
        this->add_parameter("ram_id", 1); 
        this->add_parameter("depth", depth);   
        this->add_parameter("op_width", width); 
        this->add_parameter("width", width);   
        this->add_parameter("addr_w", addr_w);   
        this->add_parameter("rst_ph", rst_ph);
        this->add_parameter("nopreload", 0);
        elaborate_foreign_module(hdl_name);
      }
  
  ~ccs_UserMemory_wrapper() {}
};

// Read-only transactor
template < int depth, int width, int addr_w, int rst_ph, bool nopreload>
  class ccs_ramifc_w_handshake_r_trans_rsc : public mc_wire_trans_rsc_base<width,depth>
{
 public:
  typedef mc_wire_trans_rsc_base<width,depth> base;
  MC_EXPOSE_NAMES_OF_BASE(base);

  enum { COLS = base::COLS };
  enum { DRV = depth}; // driving value is stored at row position depth of data

  enum XferStateT {
    XferNoState = 0,
    XferRead    = 1,
    XferWrite   = 2,
    XferWait    = 3,
    XferWaitRst = 4
  };

#ifdef DBG_CB
  static std::string getXferStr( XferStateT xferState)
  {
    std::string xfStr("None");
    switch (xferState) {
    case XferNoState: xfStr = "XferNoState"; break;
    case XferRead:    xfStr = "XferRead"; break;
    case XferWrite:   xfStr = "XferWrite"; break;
    case XferWait:    xfStr = "XferWait"; break;
    case XferWaitRst: xfStr = "XferWaitRst"; break;
    }
    return(xfStr);
  }
#endif

  // Signals from the catapult core
  sc_in< bool >            clk_int;
  sc_in< sc_logic >        rstn_int;
  sc_in< sc_logic >        clk;  // not used
  sc_in< sc_logic >        rstn; // not used
  sc_in< sc_logic >        s_re;
  sc_out< sc_logic >       s_rrdy;
  sc_in< sc_lv<addr_w> >   s_raddr;
  sc_out< sc_lv<width> >   s_din;

  // Signals going to the user memory.  Muxed with s_ and svc_ based on scvMode.
  sc_signal< sc_logic >       m_re;
  sc_signal< sc_logic >       m_rrdy;
  sc_signal< sc_lv<addr_w> >  m_raddr;
  sc_signal< sc_lv<width> >   m_din;

  // These signals used during preload/postsim scan - svcMode=0
  sc_signal< sc_logic >       scv_re;
  sc_signal< sc_lv<addr_w> >  scv_raddr;

  // These signals used during preload.  Unused in the read-only model otherwise
  sc_signal< sc_logic >       scv_we;
  sc_signal< sc_logic >       scv_wrdy;
  sc_signal< sc_lv<addr_w> >  scv_waddr;
  sc_signal< sc_lv<width> >   scv_dout;

  ccs_UserMemory_wrapper<depth, width, addr_w, rst_ph>   userSlaveInst;
  bool scvMode;  // SCVerify mode means we are preload or post sim scanout
  int readReq;
  int readResp;
  bool timeZero;

  SC_HAS_PROCESS( ccs_ramifc_w_handshake_r_trans_rsc );
  ccs_ramifc_w_handshake_r_trans_rsc(const sc_module_name& name, bool phase, double clk_skew_delay=0.0)
    : base(name, phase, clk_skew_delay)
    ,clk_int("clk_int")
    ,rstn_int("rstn_int")
    ,clk("clk")
    ,rstn("rstn")
    ,s_re("s_re")
    ,s_rrdy("s_rrdy")
    ,s_raddr("s_raddr")
    ,s_din("s_din")
    ,m_re("m_re")
    ,m_rrdy("m_rrdy")
    ,m_raddr("m_raddr")
    ,m_din("m_din")
    ,scv_re("scv_re")
    ,scv_raddr("scv_raddr")
    ,scv_we("scv_we")
    ,scv_wrdy("scv_wrdy")
    ,scv_waddr("scv_waddr")
    ,scv_dout("scv_dout")
    ,userSlaveInst("usermem", USER_MEMORY_MODULE)
  {
    userSlaveInst.clk(clk_int);
    userSlaveInst.rstn(rstn_int);
    userSlaveInst.s_re(m_re);
    userSlaveInst.s_rrdy(m_rrdy);
    userSlaveInst.s_raddr(m_raddr);
    userSlaveInst.s_din(m_din);

    userSlaveInst.s_we(scv_we);
    userSlaveInst.s_wrdy(scv_wrdy);
    userSlaveInst.s_waddr(scv_waddr);
    userSlaveInst.s_dout(scv_dout);

    MC_METHOD(muxSlaveSignals);
    sensitive << (phase ? clk_int.pos() : clk_int.neg());
    sensitive << s_re << scv_re;
    sensitive << s_raddr << scv_raddr;
    sensitive << m_rrdy;
    sensitive << m_din;

    MC_METHOD(initiate_transfer);
    this->sensitive << this->_value_changed;
    this->dont_initialize();

    MC_METHOD(at_active_clock_edge);
    sensitive << (phase ? clk_int.pos() : clk_int.neg());
    this->dont_initialize();

    MC_METHOD(updateCounts);
    sensitive << (phase ? clk_int.pos() : clk_int.neg());
    this->dont_initialize();

    if (nopreload) {
      scvMode = false;
      read_flag = true; // first transaction will be read then write
    } else {
      scvMode = true;
      read_flag = false; // initial bus transfer will be a write to the slave
    }
    transfer_started = false;
    transfer_needed = true;
    read_cnt = 0;
    write_cnt = 0;
    wait_cnt = 0;
    readReq = 0;
    readResp = 0;
    timeZero=true;

#ifdef DBG_CB
    cout << "ccs_ramifc_w_handshake_r_trans_rsc at T=" << sc_time_stamp() << std::endl;
    cout << "  depth    ="  << depth << std::endl;
    cout << "  width    ="  << width << std::endl;
    cout << "  addr_w   ="  << addr_w << std::endl;
    cout << "  rst_ph   ="  << rst_ph << std::endl;
    cout << "  nopreload="  << nopreload << std::endl;
#endif
  }

  void muxSlaveSignals() {
    if (scvMode) {
      m_re.write(scv_re.read());
      m_raddr.write(scv_raddr.read());
      s_rrdy.write(SC_LOGIC_0);
      s_din.write(0);
    } else {
      m_re.write(s_re.read());
      m_raddr.write(s_raddr.read());
      s_rrdy.write(m_rrdy.read());
      s_din.write(m_din.read());
    }
  }

  virtual void reset_stream() 
  {
#ifdef DBG_CB
    cout << "reset_stream at T=" <<  sc_time_stamp() << std::endl;
#endif
    
    if (nopreload) {
      if (!timeZero) {
        scvMode = false;
      } else {
        scvMode = true; 
      }
      read_flag = true; // next transaction will be read then write
    } else {
      scvMode = true;   
      read_flag = false; // next bus transfer will be a write to the slave
    }
    timeZero = false;
    muxSlaveSignals();
  } 

  // Count read/write requests and responses
  // The idea is that we have to wait for equivalence to transition 
  // from eval to update mode
  void updateCounts()
  {
    if ((rst_ph && rstn_int.read() == SC_LOGIC_1) || (!rst_ph && rstn_int.read() == SC_LOGIC_0))  {
      readReq = 0;
      readResp = 0;
    } else {
      if (scvMode) {
        readReq = 0;
        readResp = 0;
        return;
      }
      if (readReq > readResp) {
        if (m_rrdy.read() == SC_LOGIC_1) {
          readResp++;
          assert(readReq == readResp);
          if (!transfer_started) {
            if (m_re.read() == SC_LOGIC_1) {
              readReq++;
            }
          }
        }
      } else if (!transfer_started) {
        assert(readReq == readResp);
        if (m_re.read() == SC_LOGIC_1) {
          readReq++;
        }
      }
#ifdef DBG_CB
      cout << "ReadCounts: Req=" << readReq << "  Resp=" << readResp << " T=" << sc_time_stamp() << std::endl;
#endif
    }

    return;
  }

  // This virtual function override allows this transactor resource to delay
  // processing of the TRANSACTION_DONE signal until the mem tramsactor can 
  // perform a bus transfer to read the contents of the slave_mem for the
  // transaction that just completed BEFORE updating the transaction data.
  virtual bool bus_transfer_required() 
  {
    static sc_time firstTime;
    static bool firstValid = false;

#ifdef DBG_CB
    cout << "bus_transfer_required.  xferState=" << getXferStr(xferState)
         << " transfer_started=" << transfer_started
         << " transfer_needed=" << transfer_needed
         << " read_flag=" << read_flag
         << " read_cnt=" << read_cnt
         << " write_cnt=" << write_cnt
         << " T=" << sc_time_stamp() << std::endl;
#endif

    if (transfer_needed) {
      firstValid = false;
#ifdef DBG_CB
      cout << "  Return TRUE from bus_transfer_required" << std::endl;
#endif
      return(true);
    }
    // When multiple interface vars are mapped to one resource, we need
    // to return false once for each variable.  They all happen at the same time
    if (firstValid) {
      if ( firstTime != sc_time_stamp()) {
        // get ready for the next call to the C++ function
        transfer_needed = true;
        firstValid = false;
      }
    } else {
      firstValid = true;
      firstTime = sc_time_stamp();
    }

#ifdef DBG_CB
    cout << "  Return FALSE from bus_transfer_required" << std::endl;
#endif
    return(false);
  }

  // This method is triggered when TRANSACTION_DONE is asserted
  // read_cnt and write_cnt are 1-based.  read/writes which have been "sent"
  void initiate_transfer() {
    read_cnt = 0; // For a read-from-slave/core, m_rrdy is low.  When m_re is asserted and address
                   // is presented - both are held steady until m_rrdy is then asserted.
                   // The next address is then presented....
                   // Then read_cnt will eventually count up to 'depth'.

    if (xferState != XferNoState) {
      if (xferState == XferWaitRst) {
        transfer_needed = true;
        xferState = XferNoState;
      } else {
        // probably an error of some sort going on here
        cout << "Warning: ccs_ramifc_w_handshake_r_trans_rsc in odd state when transfer intiated!" << std::endl;
      }
    }

    write_cnt = 0;
    transfer_started = true;
    // The userMem read is eaither dense (every cycle) or sparse/delayed
    // In the delayed (stall) case, cant switch mode until final data rcvd by the core
    if (m_rrdy.read() == SC_LOGIC_1) {
      scvMode = true;
      muxSlaveSignals();
    }

#ifdef DBG_CB
    cout << "initiate_transfer.  xferState=" << getXferStr(xferState)
         << " read_flag=" << read_flag
         << "transfer_needed=" << transfer_needed
         << " readReq=" << readReq
         << " readResp=" << readResp
         << " T=" << sc_time_stamp() << std::endl;
#endif
  }


  void at_active_clock_edge() {
    if ((rst_ph && rstn_int.read() == SC_LOGIC_1) || (!rst_ph && rstn_int.read() == SC_LOGIC_0))  {
      scv_we.write(SC_LOGIC_0);
      scv_waddr.write(0);
      scv_dout.write(0);
      scv_re.write(SC_LOGIC_0);
      scv_raddr.write(0);

      xferState = XferNoState;
      read_cnt = 0;
      write_cnt = 0;

      if (nopreload) {
        // first transaction will be read then write after first compute
        scvMode = false;
      } else {
        // initial bus transfer will be a write to the slave
        scvMode = true;        
      }   
      muxSlaveSignals();
    } else {
#ifdef DBG_CB
      cout << "at_active_clk.  xferState=" << getXferStr(xferState)
           << " read_flag=" << read_flag
           << " read_cnt=" << read_cnt 
           << " current_out_row=" << this->get_current_out_row()
           << " write_cnt=" << write_cnt 
           << " current_in_row=" << this->get_current_in_row() 
           << " readReq=" << readReq
           << " readResp=" << readResp
           << " T=" << sc_time_stamp() << std::endl;
#endif
      switch(xferState) {
      case XferNoState:
        if (transfer_started) {
          if (readReq != readResp) {
#ifdef DBG_CB
            cout << "  Initiate Read DELAYED until userMem read channel is idle" << std::endl;
#endif
            break;
          }
          scvMode = true;
          muxSlaveSignals();
          if (read_flag) {
#ifdef DBG_CB
            cout << "  Initiate Read.  Addr=" << this->get_current_out_row() << std::endl;
#endif
            assert(scvMode);
            scv_re.write(SC_LOGIC_1);
            xferState = XferRead;
            scv_raddr.write(this->get_current_out_row());
            this->incr_current_out_row();
            read_cnt = 1;
          } else {
            // initiate write
#ifdef DBG_CB
            cout << "  Initiate Write.  Addr=" 
                 << this->get_current_in_row() 
                 << " Data=" << this->read_row(this->get_current_in_row()) << std::endl;
#endif
            scv_waddr.write(this->get_current_in_row());
            scv_re.write(SC_LOGIC_0);
            scv_we.write(SC_LOGIC_1);
            scv_dout.write(this->read_row(this->get_current_in_row()));
            this->incr_current_in_row();
            write_cnt = 1;
            xferState = XferWrite;
          }
        }
        break;

      case XferRead:
        if (m_rrdy.read() == SC_LOGIC_1) {
          this->write_row(read_cnt-1, this->m_din.read());
#ifdef DBG_CB
          cout << "  Harvest read data. Addr=" 
               << (read_cnt-1)
               << " Data=" << this->m_din.read() << std::endl;
#endif
          if (read_cnt >= depth) { 
            // done
#ifdef DBG_CB
            cout << "  Read transitions to Write.  Write Addr=" 
                 << this->get_current_in_row() 
                 << " Data=" << this->read_row(this->get_current_in_row()) << std::endl;
#endif
            scv_re.write(SC_LOGIC_0);
            read_cnt = 0;
            write_cnt = 1;
            read_flag = false;
            
            scv_waddr.write(this->get_current_in_row());
            scv_we.write(SC_LOGIC_1);
            scv_dout.write(this->read_row(this->get_current_in_row()));
            this->incr_current_in_row();
            xferState = XferWrite;
          } else {
#ifdef DBG_CB
            cout << "  Continue Read.  Next Addr=" << this->get_current_out_row() << std::endl;
#endif
            scv_re.write(SC_LOGIC_1);
            scv_raddr.write(this->get_current_out_row());
            this->incr_current_out_row();
            read_cnt++;
          }
        } else {
#ifdef DBG_CB
          cout << "  UserMem delays during read" << std::endl;
#endif
        }
        break;
        
      case XferWrite:
        if (scv_wrdy.read() == SC_LOGIC_1) {
#ifdef DBG_CB
          cout << "  Continue Write.  Next Addr=" 
               << this->get_current_in_row() 
               << " Data=" << this->read_row(this->get_current_in_row()) << std::endl;
#endif
          scv_waddr.write(this->get_current_in_row());
          scv_we.write(SC_LOGIC_1);
          scv_dout.write(this->read_row(this->get_current_in_row()));
          this->incr_current_in_row();
          
          if (write_cnt+1 >= depth) {
#ifdef DBG_CB
            cout << "  AND Toggle from write-to-slave to read_from-slave " << std::endl;
#endif
            wait_cnt = 5;
            xferState = XferWait;
          } else {
            write_cnt++;
          }
        } else {
          // need to hold scv_we, addr, data until it is consumed
#ifdef DBG_CB
          cout << "  UserMem stalls during write" << std::endl;
#endif
        }
        break;

      case XferWait:
        // The last write has been launched - Need to extend and let it finish
        // make sure last write finishes before turning of scv_we
#ifdef DBG_CB
        cout << "  XferWait.  WaitCnt=" << wait_cnt << " T=" << sc_time_stamp() << std::endl;
#endif
        if (scv_wrdy.read() == SC_LOGIC_1) {
          scv_we.write(SC_LOGIC_0);
          if (wait_cnt <= 0) {
            read_flag = true;  // next DONE sgnal starts with read-from-slave
            read_cnt = 0;
            write_cnt = 0;
            transfer_started = false;
            transfer_needed = false;
            scvMode = false;
            xferState = XferWaitRst; 
            wait_cnt = 3;
            muxSlaveSignals();
          } else {
            wait_cnt--;
          }
        }
        break;

      case XferWaitRst:
        // Some back-to-back calls to the C function dont call bus_transfer_required.
        // Need to reset ourselves back
#ifdef DBG_CB
        cout << "  XferWaitRst.  WaitCnt=" << wait_cnt << " T=" << sc_time_stamp() << std::endl;
#endif
        if (wait_cnt <= 0) {
#ifdef DBG_CB
          cout << "  Expired(Rtl eval in progress?) - resetting transfer_needed " << std::endl;
#endif
          transfer_needed = true;
          xferState = XferNoState;
        } else {
          wait_cnt--;
        }
        break;
        
      default:
        cout << "Error:  Bad state in transactor:" << xferState << " at T=" << sc_time_stamp() << std::endl;
        break;
      }
    }
    this->exchange_value();
  }

private:
  bool transfer_started;
  bool transfer_needed;
  XferStateT xferState;  
  bool read_flag;  // reading from the slave/core, vs writing to the slave/core
  int  read_cnt;
  int  write_cnt;
  int  wait_cnt;
};


// Write-only transactor
template < int depth, int width, int addr_w, int rst_ph, bool nopreload>
  class ccs_ramifc_w_handshake_w_trans_rsc : public mc_wire_trans_rsc_base<width,depth>
{
 public:
  typedef mc_wire_trans_rsc_base<width,depth> base;
  MC_EXPOSE_NAMES_OF_BASE(base);

  enum { COLS = base::COLS };
  enum { DRV = depth}; // driving value is stored at row position depth of data

  enum XferStateT {
    XferNoState = 0,
    XferRead    = 1,
    XferWrite   = 2,
    XferWait    = 3,
    XferWaitRst = 4
  };

#ifdef DBG_CB
  static std::string getXferStr( XferStateT xferState)
  {
    std::string xfStr("None");
    switch (xferState) {
    case XferNoState: xfStr = "XferNoState"; break;
    case XferRead:    xfStr = "XferRead"; break;
    case XferWrite:   xfStr = "XferWrite"; break;
    case XferWait:    xfStr = "XferWait"; break;
    case XferWaitRst: xfStr = "XferWaitRst"; break;
    }
    return(xfStr);
  }
#endif

  // Signals from the catapult core
  sc_in< bool >            clk_int;
  sc_in< sc_logic >        rstn_int;
  sc_in< sc_logic >        clk;  // not used
  sc_in< sc_logic >        rstn; // not used
  sc_in< sc_logic >        s_we;
  sc_out< sc_logic >       s_wrdy;
  sc_in< sc_lv<addr_w> >   s_waddr;
  sc_in< sc_lv<width> >    s_dout;

  // Signals going to the user memory.  Muxed with s_ and svc_ based on scvMode.
  sc_signal< sc_logic >       m_we;
  sc_signal< sc_logic >       m_wrdy;
  sc_signal< sc_lv<addr_w> >  m_waddr;
  sc_signal< sc_lv<width> >   m_dout;

  // These signals used during preload/postsim scan - Unused in the read-only model otherwise
  sc_signal< sc_logic >       scv_re;
  sc_signal< sc_logic >       scv_rrdy;
  sc_signal< sc_lv<addr_w> >  scv_raddr;
  sc_signal< sc_lv<width> >   scv_din;

  // These signals used during eval and postsim scan based on scvMode
  sc_signal< sc_logic >       scv_we;
  sc_signal< sc_lv<addr_w> >  scv_waddr;
  sc_signal< sc_lv<width> >   scv_dout;

  ccs_UserMemory_wrapper<depth, width, addr_w, rst_ph>   userSlaveInst;

  bool scvMode;  // SCVerify mode means we are preload or post sim scanout

  SC_HAS_PROCESS( ccs_ramifc_w_handshake_w_trans_rsc );
  ccs_ramifc_w_handshake_w_trans_rsc(const sc_module_name& name, bool phase, double clk_skew_delay=0.0)
    : base(name, phase, clk_skew_delay)
    ,clk_int("clk_int")
    ,rstn_int("rstn_int")
    ,clk("clk")
    ,rstn("rstn")
    ,s_we("s_we")
    ,s_wrdy("s_wrdy")
    ,s_waddr("s_waddr")
    ,s_dout("s_dout")
    ,m_we("m_we")
    ,m_wrdy("m_wrdy")
    ,m_waddr("m_waddr")
    ,m_dout("m_din")
    ,scv_re("scv_re")
    ,scv_rrdy("scv_rrdy")
    ,scv_raddr("scv_raddr")
    ,scv_din("scv_din")
    ,scv_we("scv_we")
    ,scv_waddr("scv_waddr")
    ,scv_dout("scv_dout")
    ,userSlaveInst("usermem", USER_MEMORY_MODULE)
  {
    userSlaveInst.clk(clk_int);
    userSlaveInst.rstn(rstn_int);
    userSlaveInst.s_re(scv_re);
    userSlaveInst.s_rrdy(scv_rrdy);
    userSlaveInst.s_raddr(scv_raddr);
    userSlaveInst.s_din(scv_din);

    userSlaveInst.s_we(m_we);
    userSlaveInst.s_wrdy(m_wrdy);
    userSlaveInst.s_waddr(m_waddr);
    userSlaveInst.s_dout(m_dout);

    MC_METHOD(muxSlaveSignals);
    sensitive << (phase ? clk_int.pos() : clk_int.neg());
    sensitive << s_we << scv_we;
    sensitive << s_waddr << scv_waddr;
    sensitive << s_dout << scv_dout;
    sensitive << m_wrdy;

    MC_METHOD(initiate_transfer);
    this->sensitive << this->_value_changed;
    this->dont_initialize();

    MC_METHOD(at_active_clock_edge);
    sensitive << (phase ? clk_int.pos() : clk_int.neg());
    this->dont_initialize();

    if (nopreload) {
      scvMode = false;
      read_flag = true; // first transaction will be read then write
    } else {
      scvMode = true;  // gotta preload first
      read_flag = false; // initial bus transfer will be a write to the slave
    }
    transfer_started = false;
    transfer_needed = true;
    read_cnt = 0;
    write_cnt = 0;
    wait_cnt = 0;

#ifdef DBG_CB
    cout << "ccs_ramifc_w_handshake_w_trans_rsc at T=" << sc_time_stamp() << std::endl;
    cout << "  depth    ="  << depth << std::endl;
    cout << "  width    ="  << width << std::endl;
    cout << "  addr_w   ="  << addr_w << std::endl;
    cout << "  rst_ph   ="  << rst_ph << std::endl;
    cout << "  nopreload="  << nopreload << std::endl;
#endif
  }

  void muxSlaveSignals() {
    if (scvMode) {
      m_we.write(scv_we.read());
      m_waddr.write(scv_waddr.read());
      m_dout.write(scv_dout.read());
      s_wrdy.write(SC_LOGIC_0);
    } else {
      m_we.write(s_we.read());
      m_waddr.write(s_waddr.read());
      m_dout.write(s_dout.read());
      s_wrdy.write(m_wrdy.read());
    }
  }

  virtual void reset_stream() 
  {
#ifdef DBG_CB
    cout << "reset_stream at T=" <<  sc_time_stamp() << std::endl;
#endif
    if (nopreload) {
      read_flag = true; // next transaction will be read then write
    } else {
      read_flag = false; // next bus transfer will be a write to the slave
    }
  } 

  // This virtual function override allows this transactor resource to delay
  // processing of the TRANSACTION_DONE signal until the axi4_master can
  // perform a bus transfer to read the contents of the axi4_slave_mem for the
  // transaction that just completed BEFORE updating the transaction data.
  virtual bool bus_transfer_required() 
  {
    static sc_time firstTime;
    static bool firstValid = false;

#ifdef DBG_CB
    cout << "bus_transfer_required.  xferState=" << getXferStr(xferState)
         << " transfer_started=" << transfer_started
         << " transfer_needed=" << transfer_needed
         << " read_flag=" << read_flag
         << " read_cnt=" << read_cnt
         << " write_cnt=" << write_cnt
         << " T=" << sc_time_stamp() << std::endl;
#endif
    if (transfer_needed) {
      firstValid = false;
      return(true);
    }
    // When multiple interface vars are mapped to one resource, we need
    // to return false once for each variable.  They all happen at the same time
    if (firstValid) {
      if ( firstTime != sc_time_stamp()) {
        // get ready for the next call to the C++ function
        transfer_needed = true;
        firstValid = false;
      }
    } else {
      firstValid = true;
      firstTime = sc_time_stamp();
    }
    
    return(false);
  }

  // This method is triggered when TRANSACTION_DONE is asserted
  // read_cnt and write_cnt are 1-based.  read/writes which have been "sent"
  void initiate_transfer() {
    read_cnt = 0; // For a read-from-slave/core, m_rrdy is low.  When m_re is asserted and address
                   // is presented - both are held steady until m_rrdy is then asserted.
                   // The next address is then presented....
                   // Then read_cnt will eventually count up to 'depth'.

    if (xferState != XferNoState) {
      if (xferState == XferWaitRst) {
        transfer_needed = true;
        xferState = XferNoState;
      } else {
        // probably an error of some sort going on here
        cout << "Warning: ccs_ramifc_w_handshake_r_trans_rsc in odd state when transfer intiated!" << std::endl;
      }
    }

    write_cnt = 0;
    transfer_started = true;
    if (m_wrdy.read() == SC_LOGIC_1) {
      scvMode = true;
      muxSlaveSignals();
    }

#ifdef DBG_CB
    cout << "initiate_transfer.  xferState=" << getXferStr(xferState)
         << " read_flag=" << read_flag
         << "transfer_needed=" << transfer_needed
         << " T=" << sc_time_stamp() << std::endl;
#endif
  }

  void at_active_clock_edge() {
    if ((rst_ph && rstn_int.read() == SC_LOGIC_1) || (!rst_ph && rstn_int.read() == SC_LOGIC_0))  {
      scv_we.write(SC_LOGIC_0);
      scv_waddr.write(0);
      scv_dout.write(0);
      scv_re.write(SC_LOGIC_0);
      scv_raddr.write(0);

      xferState = XferNoState;
      read_cnt = 0;
      write_cnt = 0;

      if (nopreload) {
        // first transaction will be read then write
        scvMode = false;
      } else {
        // initial bus transfer will be a write to the slave
        scvMode = true;        
      }   
      muxSlaveSignals();
    } else {
#ifdef DBG_CB
      cout << "at_active_clk.  xferState=" << getXferStr(xferState)
           << " read_flag=" << read_flag
           << " read_cnt=" << read_cnt 
           << " current_out_row=" << this->get_current_out_row()
           << " write_cnt=" << write_cnt 
           << " current_in_row=" << this->get_current_in_row() 
           << " T=" << sc_time_stamp() << std::endl;
#endif
      switch(xferState) {
      case XferNoState:
        if (transfer_started) {
          if (m_wrdy.read() == SC_LOGIC_0) {
#ifdef DBG_CB
            cout << "  Initiate Read DELAYED until userMem write channel is idle" << std::endl;
#endif
            break;
          }
          scvMode = true;
          muxSlaveSignals();

          if (read_flag) {
#ifdef DBG_CB
            cout << "  Initiate Read.  Addr=" << this->get_current_out_row() << std::endl;
#endif
            assert(scvMode);
            scv_re.write(SC_LOGIC_1);
            xferState = XferRead;
            scv_raddr.write(this->get_current_out_row());
            this->incr_current_out_row();
            read_cnt = 1;
          } else {
            // initiate write
#ifdef DBG_CB
            cout << "  Initiate Write.  Addr=" 
                 << this->get_current_in_row() 
                 << " Data=" << this->read_row(this->get_current_in_row()) << std::endl;
#endif
            scv_re.write(SC_LOGIC_0);
            scv_waddr.write(this->get_current_in_row());
            scv_we.write(SC_LOGIC_1);
            scv_dout.write(this->read_row(this->get_current_in_row()));
            this->incr_current_in_row();
            write_cnt = 1;
            xferState = XferWrite;
          }
        }
        break;

      case XferRead:
        if (scv_rrdy.read() == SC_LOGIC_1) {
          this->write_row(read_cnt-1, this->scv_din.read());
#ifdef DBG_CB
          cout << "  Harvest read data. Addr=" 
               << (read_cnt-1)
               << " Data=" << this->scv_din.read() << std::endl;
#endif
          if (read_cnt >= depth) { 
            // done
#ifdef DBG_CB
            cout << "  Read transitions to Write.  Write Addr=" 
                 << this->get_current_in_row() 
                 << " Data=" << this->read_row(this->get_current_in_row()) << std::endl;
#endif
            scv_re.write(SC_LOGIC_0);
            read_cnt = 0;
            write_cnt = 1;
            read_flag = false;
            
            scv_waddr.write(this->get_current_in_row());
            scv_we.write(SC_LOGIC_1);
            scv_dout.write(this->read_row(this->get_current_in_row()));
            this->incr_current_in_row();
            xferState = XferWrite;
          } else {
#ifdef DBG_CB
            cout << "  Continue Read.  Next Addr=" << this->get_current_out_row() << std::endl;
#endif
            scv_re.write(SC_LOGIC_1);
            scv_raddr.write(this->get_current_out_row());
            this->incr_current_out_row();
            read_cnt++;
          }
        } else {
#ifdef DBG_CB
          cout << "  UserMem delays during read" << std::endl;
#endif
        }
        break;
        
      case XferWrite:
        if (m_wrdy.read() == SC_LOGIC_1) {
#ifdef DBG_CB
          cout << "  Continue Write.  Next Addr=" 
               << this->get_current_in_row() 
               << " Data=" << this->read_row(this->get_current_in_row()) << std::endl;
#endif
          scv_waddr.write(this->get_current_in_row());
          scv_we.write(SC_LOGIC_1);
          scv_dout.write(this->read_row(this->get_current_in_row()));
          this->incr_current_in_row();
          
          if (write_cnt+1 >= depth) {
#ifdef DBG_CB
            cout << "  AND Toggle from write-to-slave to read_from-slave " << std::endl;
#endif
            wait_cnt = 5;
            xferState = XferWait;
          } else {
            //this->incr_current_in_row();
            write_cnt++;
          }
        } else {
          // need to hold scv_we, addr, data until it is consumed
#ifdef DBG_CB
          cout << "  UserMem stalls during write" << std::endl;
#endif
        }
        break;

      case XferWait:
        // The last write has been launched - Need to extend and let it finish
        // make sure last write finishes before turning of scv_we
#ifdef DBG_CB
        cout << "  XferWait.  WaitCnt=" << wait_cnt << " T=" << sc_time_stamp() << std::endl;
#endif
        if (m_wrdy.read() == SC_LOGIC_1) {
          scv_we.write(SC_LOGIC_0);
          if (wait_cnt <= 0) {
            read_flag = true;  // next DONE sgnal starts with read-from-slave
            read_cnt = 0;
            write_cnt = 0;
            transfer_started = false;
            transfer_needed = false;
            scvMode = false;
            xferState = XferWaitRst; 
            wait_cnt = 3;
            muxSlaveSignals();
          } else {
            wait_cnt--;
          }
        }
        break;

      case XferWaitRst:
        // Some back-to-back calls to the C function dont call bus_transfer_required.
        // Need to reset ourselves back
#ifdef DBG_CB
        cout << "  XferWaitRst.  WaitCnt=" << wait_cnt << " T=" << sc_time_stamp() << std::endl;
#endif
        if (wait_cnt <= 0) {
#ifdef DBG_CB
          cout << "  Expired(Rtl eval in progress?) - resetting transfer_needed " << std::endl;
#endif
          transfer_needed = true;
          xferState = XferNoState;
        } else {
          wait_cnt--;
        }
        break;
        
      default:
        cout << "Error:  Bad state in transactor:" << xferState << " at T=" << sc_time_stamp() << std::endl;
        break;
      }
    }
    this->exchange_value();
  }

private:
  bool transfer_started;
  bool transfer_needed;
  XferStateT xferState;  
  bool read_flag;  // reading from the slave/core, vs writing to the slave/core
  int  read_cnt;
  int  write_cnt;
  int  wait_cnt;
};


#endif // ifndef __INCLUDED_ccs_ram_w_handshake_trans_rsc_H__


