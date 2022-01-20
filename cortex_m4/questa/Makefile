

#key parameters for the Makefile
#
# ACCEL=  should be set to mac, null, master_par[1-5], slave_par[1-5]
# MASTER= must be defined if ACCEL is master_par[1-5]
# TB_MEM= if defined moves large intermediate memory from design into testbench
# FSDB= define if you want to create an FSDB from the run
#
LOCATION = "MENTOR"
#LOCATION = "GLOBAL_FOUNDRIES"

#====== GF Location Specific Settings ============

ifeq ($(LOCATION), "GLOBAL_FOUNDRIES")

export QUESTA_HOME      ?= /tool/eda/apps/mentorQuestaUltra/10.6c/questasim/
export VIS_HOME         ?= /u/release/2020.2/visualizer
export CODELINK_HOME    ?= /project/codelink_qa/codelink_homes/codelink_home_2020.1_ixl
export M4_SOURCE_DIR    ?= /proj/cad_scratch6/prajput/mentor/m4/cortex_m4/arm_cortex_m4/logical/
export CATAPULT_HOME    ?= /tool/eda/apps/mentorCatapult/10.5/Mgc_home/
export CODEBENCH_HOME   ?= /proj/cad_arm_14lpp/knaraya2/m4_sample_rundir/gcc-arm-none-eabi-5_4-2016q3
export VERDI_HOME       ?=
export ARM_LIB          ?= 

endif

#====== Location Specific Settings ============

ifeq ($(LOCATION), "MENTOR")

export QUESTA_HOME      ?= /u/release/2020.2/questasim
export VIS_HOME         ?= /u/release/2020.2/visualizer
export CODELINK_HOME    ?= /project/codelink_qa/codelink_homes/codelink_home_2020.1_aol
export M4_SOURCE_DIR    ?= /home/russk/gf_staging/from_gf/arm_cortex_m4/logical
export CATAPULT_HOME    ?= /wv/hlsb/CATAPULT/10.5a/PRODUCTION/aol/Mgc_home
export CODEBENCH_HOME   ?= /home/russk/tools/compilers/gcc_arm32/gcc-arm-none-eabi-9-2019-q4-major
export VERDI_HOME       ?= /wv/hlstools/Verdi/O-2018.09/verdi/Verdi_O-2018.09
export ARM_LIB          ?= /home/russk/gf_staging/mnist_small/arm_lib

endif

#====== Run Specific Settings =================
#       set on the 'make' command line

export ACCEL            ?= null
export PROGRAM          ?= hello

#=============================================

ifdef OPT
GCC_OPT = "OPT=true"
endif

ifdef FSDB
FSDB_OPT = dump
endif

ifdef MASTER
VSIM_MASTER = "+define+MASTER"
endif

ifdef TB_MEM
WEIGHT_MEMORY = tbench.tbench_memory
TB_MEM_OPT = +define+TB_MEM
else
WEIGHT_MEMORY = tbench.top.mem1
endif

PAR ?= 1

export ACCEL_PATH        = ../accelerators/$(ACCEL)

export PATH             := $(QUESTA_HOME)/bin:$(VIS_HOME)/bin:$(PATH)
export LD_LIBRARY_PATH  := $(LD_LIBRARY_PATH):$(VERDI_HOME)/share/PLI/lib/LINUX64

VLOG           = vlog
VSIM           = vsim
VMAP           = vmap
VLIB           = vlib
VOPT           = vopt
SCCOM          = sccom

VLOG_FLAGS     = -work $(work) +acc=all -64 +define+SIMULATION $(TB_MEM_OPT)
VIS_FLAGS      = -work $(work) -t 1ps -visualizer -64 -qwavedb=+signal+report+memory=1024+transaction+class $(TB_MEM_OPT)
VSIM_FLAGS     = -work $(work) -L $(ARM_LIB) -t 1ps -voptargs="+acc" -do run.do -64 -pli $(VERDI_HOME)/share/PLI/MODELSIM/linux64/novas_fli.so +define+CODE_MEMORY_FILE="../sw/$(PROGRAM)/$(PROGRAM).mem"
VOPT_VIS_FLAGS = -undefsyms=verbose -debug +designfile 
VOPT_FLAGS     = -g +acc=all +designfile -debug
SCCOM_FLAGS    = -g -O0 $(INCLUDES) -std=c++11
QUIET_FLAG     = --no-print-directory


export TERMINAL      = ../terminal
export MEMORY_IMAGE  = ../sw/$(PROGRAM)/$(PROGRAM).mem

terminal       = terminal.so

hw_sources     = ../hw
marker_dir     = $(markers)/maker.mark

# markers

markers        = marker_files
build          = $(markers)/build.mark
build_vis      = $(markers)/build_vis.mark
work_lib       = $(markers)/work.mark
tbench         = $(markers)/tbench.sv.mark
#cortex_m4      = $(markers)/cortex_m4.mark
codelink       = $(markers)/codelink.mark
accelerator    = $(markers)/accelerator.mark

hdl_source_markers =                     \
		 $(markers)/blgen.sv.mark           \
		 $(markers)/char_in.sv.mark         \
		 $(markers)/char_out.sv.mark        \
		 $(markers)/sram.sv.mark            \
		 $(markers)/bus_fifo.sv.mark        \
		 $(markers)/switch_ahb.sv.mark      \
		 $(markers)/tbench.sv.mark          \
		 $(markers)/timer.sv.mark           \
		 $(markers)/top.sv.mark             \
		 $(markers)/uart.sv.mark            \
		 $(markers)/dump.v.mark

m4_source      = $(M4_SOURCE_DIR)/cortexm4/verilog/CORTEXM4.v
design         = $(tbench)
opt_object     = tbench_opt
vis_opt_object = tbench_opt_vis

work           = ./work

CRUFT          = transcript              \
		 *.wlf                   \
		 wlf*                    \
		 modelsim.ini            \
		 codelink.ini            \
		 *stacktrace*            \
		 qwave.db                \
		 .visualizer             \
		 visualizer.log          \
		 design.bin              \
		 make.out                \
                 codelink_replay.rlf     \
		 .cdl_top.jdb            \
		 .codelink_logs          \
		 .codelink_out           \
                 run.do                  \
		 novas_dump.log          \
		 novas.fsdb              \
		 core.*

.PHONY: run

#====== Questa target ==============

.PHONY: run_questa

run_questa: $(marker_dir) $(work_lib) $(hdl_source_markers) $(cortex_m4) $(accelerator) $(terminal) run.do software
	@echo VSIM    tbench $(PROGRAM) $(FSDB_OPT)
	@echo "Program = $(PROGRAM) ACCEL = $(ACCEL) "
	@ln -sf ../sw/$(PROGRAM)/$(PROGRAM).mem code.mem
	@ln -sf ../../python_mnist/fixed_weights_$(PAR).mem weights.mem
	@$(VSIM) $(VSIM_FLAGS) tbench $(FSDB_OPT) -sv_lib terminal >> make.out
	@rm -f code.mem
	@rm -f weights.mem

$(build): $(marker_dir) $(work_lib) $(hdl_source_markers) $(cortex_m4) $(accelerator) $(terminal) 
	@echo VOPT    tbench
	@$(VOPT) $(VOPT_FLAGS) tbench -o $(opt_object) >> make.out
	@touch $(build)

#====== Visualizer target ==========

.PHONY: run_vis

run_vis: $(build_vis) run.do software
	@echo VSIM    $(vis_opt_object)
	@$(VSIM) $(VIS_FLAGS) $(vis_opt_object) -do run.do -sv_lib terminal >> make.out

$(build_vis): $(marker_dir) $(work_lib) $(hdl_source_markers) $(cortex_m4) $(accelerator) $(terminal) 
	@echo VOPT    tbench $(FSDB_OPT)
	@$(VOPT) $(VOPT_VIS_FLAGS) tbench $(FSDB_OPT) -64 -o $(vis_opt_object) >> make.out
	@touch $(build_vis)

#====== Codelink target ============

.PHONY: run_codelink

run_codelink: $(codelink) $(build) run.do codelink.ini software
	$(VSIM) $(VSIM_FLAGS) tbench -sv_lib terminal \
		-foreign "codelink $(CODELINK_HOME)/lib/codelink.so" \
		+codelink_cpulabel=Top

#====== M4 processor ===============
# ARM IP should be precompiled into $(ARM_LIB)
#
#$(cortex_m4): $(marker_dir) $(work_lib) $(hw_sources)/compile.vc $(m4_source)
#	@echo VLOG    $(m4_source)
#	@$(VLOG) $(VLOG_FLAGS) -f $(hw_sources)/compile.vc $(m4_source) >> make.out
#	@touch $(cortex_m4)
#
#====== Accelerator =================

$(accelerator):
	@echo "Making accelerator"
	@make -C $(ACCEL_PATH)/src $(QUIET_FLAG)
	@echo $(VLOG) $(VLOG_FLAGS) $(ACCEL_PATH)/hw/cat_accel.v
	@$(VLOG) $(VLOG_FLAGS) $(ACCEL_PATH)/hw/cat_accel.v >> make.out
	@echo $(VLOG) $(VLOG_FLAGS) $(ACCEL_PATH)/hw/$(ACCEL).v
	@$(VLOG) $(VLOG_FLAGS) $(ACCEL_PATH)/hw/$(ACCEL).v >> make.out
	@touch $(accelerator)

#====== Codelink ====================
#       in simulation software debug

$(codelink): $(marker_dir) $(work_lib) $(CODELINK_HOME)/vsim/codelink_cpu_CORTEXM4_r0_v2.sv
	@echo VLOG $(CODELINK_HOME)/vsim/codelink_cpu_CORTEXM4_r0_v2.sv
	@$(VLOG) $(VLOG_FLAGS) $(CODELINK_HOME)/vsim/codelink_cpu_CORTEXM4_r0_v2.sv +define+CPU_PATH=tbench.top.cpu  >> make.out
	@echo VLOG ../hw/tbench.v +define+CODELINK
	@$(VLOG) $(VLOG_FLAGS) ../hw/tbench.sv +define+CODELINK >> make.out
	@touch $(tbench)
	@touch $(codelink)

#====== Generic compile rules ======
 
$(object_dir)/%.o: %.cpp
	@echo CC      $<
	@$(GCC) $(CFLAGS) $(INCLUDES) -c -o $@ $< >> make.out

$(markers)/%.sv.mark: $(hw_sources)/%.sv
	@echo VLOG    $<
	@$(VLOG) $(VLOG_FLAGS) $(VSIM_MASTER) $< >> make.out
	@touch $@

$(markers)/%.v.mark: $(hw_sources)/%.v
	@echo VLOG    $<
	@$(VLOG) $(VLOG_FLAGS) $(VSIM_MASTER) $< >> make.out
	@touch $@

$(markers)/%.sc.mark: %.cpp
	@echo SCCOM   $<
	@$(SCCOM) $(SCCOM_FLAGS) $(DEFINES) -D QUESTA -D MATCHLIB $<
	@touch $@

#====== Working directories ========

$(marker_dir):
	@echo MKDIR   $@
	@mkdir $(markers) >> make.out
	@touch $(marker_dir)

$(work_lib): $(marker_dir)
	@echo VLIB    $(work)
	@$(VLIB) $(work) >> make.out
	@echo VMAP    -work $(work)
	@$(VMAP) work $(work) >> make.out
	@touch $(work_lib)

#====== Support programs ===========

$(terminal): $(TERMINAL)/dpi/terminal.c $(TERMINAL)/bin/terminal_emulator
	@echo CC      $<
	@$(CC) -m64 -DVSIM -fPIC -shared -o $(terminal) $(TERMINAL)/dpi/terminal.c -I $(QUESTA_HOME)/include -I ./

$(TERMINAL)/bin/terminal_emulator: $(TERMINAL)/dpi/terminal_emulator.c
	@echo CC      $<
	@$(CC) -m64 -o $(TERMINAL)/bin/terminal_emulator $(TERMINAL)/dpi/terminal_emulator.c -lpthread >> make.out


#====== Questa run script ==========

run.do: 
	@echo CREATE   run.do
	@echo "mem load -filldata 0 tbench.top.mem0.byte0.mem                                                     " >  run.do
	@echo "mem load -filldata 0 tbench.top.mem0.byte1.mem                                                     " >> run.do
	@echo "mem load -filldata 0 tbench.top.mem0.byte2.mem                                                     " >> run.do
	@echo "mem load -filldata 0 tbench.top.mem0.byte3.mem                                                     " >> run.do
	@echo "                                                                                                   " >> run.do
	@echo "mem load -infile \$$env(MEMORY_IMAGE).byte0 -format hex tbench.top.mem0.byte0.mem                  " >> run.do
	@echo "mem load -infile \$$env(MEMORY_IMAGE).byte1 -format hex tbench.top.mem0.byte1.mem                  " >> run.do
	@echo "mem load -infile \$$env(MEMORY_IMAGE).byte2 -format hex tbench.top.mem0.byte2.mem                  " >> run.do
	@echo "mem load -infile \$$env(MEMORY_IMAGE).byte3 -format hex tbench.top.mem0.byte3.mem                  " >> run.do
	@echo "                                                                                                   " >> run.do
	@echo "mem load -filldata 0 $(WEIGHT_MEMORY).byte0.mem                                                    " >> run.do
	@echo "mem load -filldata 0 $(WEIGHT_MEMORY).byte1.mem                                                    " >> run.do
	@echo "mem load -filldata 0 $(WEIGHT_MEMORY).byte2.mem                                                    " >> run.do
	@echo "mem load -filldata 0 $(WEIGHT_MEMORY).byte3.mem                                                    " >> run.do
	@echo "                                                                                                   " >> run.do
	@echo "mem load -infile ../weights/fixed_weights_$(PAR).byte0.mem -format hex $(WEIGHT_MEMORY).byte0.mem  " >> run.do
	@echo "mem load -infile ../weights/fixed_weights_$(PAR).byte1.mem -format hex $(WEIGHT_MEMORY).byte1.mem  " >> run.do
	@echo "mem load -infile ../weights/fixed_weights_$(PAR).byte2.mem -format hex $(WEIGHT_MEMORY).byte2.mem  " >> run.do
	@echo "mem load -infile ../weights/fixed_weights_$(PAR).byte3.mem -format hex $(WEIGHT_MEMORY).byte3.mem  " >> run.do
	@echo "                                                                                                   " >> run.do
	@echo "add wave -r /*                                                                                     " >> run.do
	@echo "run 3000 ms                                                                                        " >> run.do
	@echo "                                                                                                   " >> run.do


#====== Codelink.ini file ==========

codelink.ini:
	@echo CREATE codelink.ini
	@echo "[Top]                                       " >  codelink.ini
	@echo "inst_path = tbench.monitor                  " >> codelink.ini
	@echo "elf_file = ../sw/$(PROGRAM)/\$${PROGRAM}    " >> codelink.ini
	@echo "                                            " >> codelink.ini
	@echo "proc_type = armcortex_m4rtl                 " >> codelink.ini

#====== Software image =============

.PHONY: software

software:
	@echo "Building program: $(PROGRAM)"
	@make -C ../sw/$(PROGRAM) PROGRAM=$(PROGRAM) ACCEL=$(ACCEL) $(GCC_OPT) $(QUIET_FLAG)

program_image.mem: ../sw/$(PROGRAM)/$(PROGRAM).mem 
	@ln -s program_image.mem ../sw/$(PROGRAM)/$(PROGRAM).mem

#====== Clean up ===================

.PHONY: clean

clean:
	@echo RM      targets and cruft
	@rm -rf $(target)        \
                $(work)          \
                $(markers)       \
                $(object_dir)    \
                $(CRUFT)         \
                $(terminal)      \
		$(TERMINAL)/bin/terminal_emulator
	@make -C ../sw/$(PROGRAM) clean $(QUIET_FLAG)
