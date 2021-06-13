
LOCATION = "MENTOR"
#LOCATION = "GLOBAL_FOUNDRIES"

#====== GF Location Specific Settings ============

ifeq ($(LOCATION), "GLOBAL_FOUNDRIES")

CODEBENCH_HOME  ?= /proj/cad_arm_14lpp/knaraya2/m4_sample_rundir/gcc-arm-none-eabi-5_4-2016q3

endif

#====== Location Specific Settings ============

ifeq ($(LOCATION), "MENTOR")

#CODEBENCH_HOME  ?= /home/russk/accelerator/cat_accel_design/tools/CodeSourcery/Sourcery_CodeBench_for_ARM_EABI
CODEBENCH_HOME  ?= /home/russk/accelerator/cat_accel_design/tools/gcc-arm-none-eabi-9-2019-q4-major

endif

#==============================================

ifdef OPT
GCC_DEBUG_FLAGS = -g -O3
else 
GCC_DEBUG_FLAGS = -g3 -O0
endif

PAR_IN           ?= 1
PROGRAM          ?= master_par$(PAR_IN)
ACCEL            ?= $(PROGRAM)

ACCELERATOR_PATH ?= ../../accelerators

CC        = $(CODEBENCH_HOME)/bin/arm-none-eabi-gcc
LD        = $(CODEBENCH_HOME)/bin/arm-none-eabi-gcc
ASM       = $(CODEBENCH_HOME)/bin/arm-none-eabi-gcc
OBJCOPY   = $(CODEBENCH_HOME)/bin/arm-none-eabi-objcopy

DEFINES   = -D WEIGHT_MEMORY -D FIXED_POINT -D ARM -D MASTER -D M3 -D PAR_IN=$(PAR_IN)
INCLUDES  = -I . -I $(ACCELERATOR_PATH)/$(ACCEL)/sw -I ../../../python_mnist

CFLAGS    = -c $(GCC_DEBUG_FLAGS) $(DEFINES) -mthumb -mcpu=cortex-m4 $(INCLUDES)

ASMFLAGS  = -x assembler-with-cpp $(CFLAGS)
LDFLAGS   = -mcpu=cortex-m3 -mthumb -Xlinker 
OBJS      = $(PROGRAM).o startup_ARMCM4.o console.o syscalls.o

$(PROGRAM).mem: $(PROGRAM) Makefile
	@echo OBJCOPY-ARM $(PROGRAM) -o $(PROGRAM).bin
	@$(OBJCOPY) -O binary $(PROGRAM) $(PROGRAM).bin 
	@echo "OD $(PROGRAM).bin -o $(PROGRAM).mem"
	@od -v --width=4 -tx4 $(PROGRAM).bin | cut -c9- > $(PROGRAM).mem 
	@echo "Creating byte memory images... "
	@cut $(PROGRAM).mem -c 1,2 > $(PROGRAM).mem.byte3
	@cut $(PROGRAM).mem -c 3,4 > $(PROGRAM).mem.byte2
	@cut $(PROGRAM).mem -c 5,6 > $(PROGRAM).mem.byte1
	@cut $(PROGRAM).mem -c 7,8 > $(PROGRAM).mem.byte0


$(PROGRAM): $(OBJS) Makefile
	@echo LD-ARM $(OBJS) -o $(PROGRAM)
	@$(LD)         \
        $(LDFLAGS)         \
        -Map=$(PROGRAM).map  \
        -T gcc.ld    \
        -o $(PROGRAM)        \
        $(OBJS)             \
        -L $(CODEBENCH_HOME)/arm-none-eabi/lib/armv6-m -lm >> make.out

system_init.o: system_init.S Makefile
	@echo ASM-ARM system_init.S -o system_init.o
	@$(ASM) $(ASMFLAGS) -o system_init.o system_init.S >> make.out

ew2015-reset-ram.o: ew2015-reset-ram.S Makefile
	@echo ASM-ARM ew2015-reset-ram.S -o ew2015-reset-ram.o
	@$(ASM) $(ASMFLAGS) -o ew2015-reset-ram.o ew2015-reset-ram.S >> make.out

startup_ARMCM4.o: startup_ARMCM4.S Makefile
	@echo ASM-ARM startup_ARMCM4.S -o $@
	@$(ASM) $(ASMFLAGS) -o $@ startup_ARMCM4.S >> make.out

$(PROGRAM).o: $(PROGRAM).c Makefile
	@echo GCC-ARM $< -o $@
	$(CC) $(CFLAGS) -o $(PROGRAM).o $(PROGRAM).c  >> make.out

%.o: %.c 
	@echo GCC-ARM $< -o $@
	@$(CC) $(CFLAGS) -o $@ $< >> make.out

clean:
	@echo RM -rf targets and cruft
	@rm -rf $(OBJS)        \
		$(PROGRAM)     \
		$(PROGRAM).mem \
		$(PROGRAM).map \
		$(PROGRAM).bin \
		$(PROGRAM).o   \
		$(PROGRAM).map \
		$(PROGRAM).bin \
		$(PROGRAM).mem \
		make.out

