
#====== Location Specific Settings ============

#GCC_HOME  ?= /home/russk/accelerator/cat_accel_design/tools/CodeSourcery/Sourcery_CodeBench_for_ARM_EABI
#GCC_HOME  ?= /home/russk/accelerator/cat_accel_design/tools/gcc-arm-none-eabi-9-2019-q4-major
GCC_HOME  ?= /home/russk/accelerator/cat_accel_design/tools/gcc-arm-9.2-2019.12-x86_64-aarch64-none-elf

#==============================================

ifdef OPT
GCC_DEBUG_FLAGS = -g -O3
else 
GCC_DEBUG_FLAGS = -g3 -O0
endif

PAR         ?= 4
PROGRAM     ?= master_par$(PAR)
ACCEL	    ?= $(PROGRAM)

ACCELERATOR_PATH ?= ../../accelerators

DEFINES   = -D WEIGHT_MEMORY -D FIXED_POINT -D ARM -D MASTER -D A53 -D PAR_IN=$(PAR)
INCLUDES  = -I . -I $(ACCELERATOR_PATH)/$(ACCEL)/sw -I ../../../python_mnist

CC	  = $(GCC_HOME)/bin/aarch64-none-elf-gcc
LD	  = $(GCC_HOME)/bin/aarch64-none-elf-gcc
ASM       = $(GCC_HOME)/bin/aarch64-none-elf-gcc
OBJCOPY   = $(GCC_HOME)/bin/aarch64-none-elf-objcopy

CFLAGS    = -c $(GCC_DEBUG_FLAGS) -mcpu=cortex-a53 -I$(ACCELERATOR_PATH)/$(ACCEL)/sw $(DEFINES) $(INCLUDES)

ASMFLAGS  = -x assembler-with-cpp $(CFLAGS)
LDFLAGS   = -mcpu=cortex-a53 -Xlinker 
OBJS      = $(PROGRAM).o arm_start.o console.o syscalls.o

$(PROGRAM).mem: $(PROGRAM) Makefile
	@echo OBJCOPY-ARM $(PROGRAM) -o $(PROGRAM).bin
	@$(OBJCOPY) -O binary $(PROGRAM) $(PROGRAM).bin 
	@echo "OD $(PROGRAM).bin -o $(PROGRAM).mem"
	@od -v --width=16 -tx8 $(PROGRAM).bin | cut -c9-24  > $(PROGRAM)_a.temp
	@od -v --width=16 -tx8 $(PROGRAM).bin | cut -c26-41 > $(PROGRAM)_b.temp
	@paste -d'\0' $(PROGRAM)_b.temp $(PROGRAM)_a.temp > $(PROGRAM).mem 
	@rm -f $(PROGRAM)_a.temp $(PROGRAM)_b.temp
	@echo "Creating byte memory images... "
	@cut $(PROGRAM).mem -c 1,2   > $(PROGRAM).byte15.mem
	@cut $(PROGRAM).mem -c 3,4   > $(PROGRAM).byte14.mem
	@cut $(PROGRAM).mem -c 5,6   > $(PROGRAM).byte13.mem
	@cut $(PROGRAM).mem -c 7,8   > $(PROGRAM).byte12.mem
	@cut $(PROGRAM).mem -c 9,10  > $(PROGRAM).byte11.mem
	@cut $(PROGRAM).mem -c 11,12 > $(PROGRAM).byte10.mem
	@cut $(PROGRAM).mem -c 13,14 > $(PROGRAM).byte9.mem
	@cut $(PROGRAM).mem -c 15,16 > $(PROGRAM).byte8.mem
	@cut $(PROGRAM).mem -c 17,18 > $(PROGRAM).byte7.mem
	@cut $(PROGRAM).mem -c 19,20 > $(PROGRAM).byte6.mem
	@cut $(PROGRAM).mem -c 21,22 > $(PROGRAM).byte5.mem
	@cut $(PROGRAM).mem -c 23,24 > $(PROGRAM).byte4.mem
	@cut $(PROGRAM).mem -c 25,26 > $(PROGRAM).byte3.mem
	@cut $(PROGRAM).mem -c 27,28 > $(PROGRAM).byte2.mem
	@cut $(PROGRAM).mem -c 29,30 > $(PROGRAM).byte1.mem
	@cut $(PROGRAM).mem -c 31,32 > $(PROGRAM).byte0.mem


$(PROGRAM): $(OBJS) Makefile
	@echo LD-ARM $(OBJS) -o $(PROGRAM)
	$(LD)	 \
	$(LDFLAGS)	 \
	-Map=$(PROGRAM).map  \
	-T arm.ld    \
	-o $(PROGRAM)	\
	$(OBJS)	     \
	-L $(GCC_HOME)/arm-none-elf/lib/gcc/aarch64-none-elf/9.2.1 -lm >> make.out

arm_start.o: arm_start.s Makefile
	@echo "ASM-ARM !< -o !@ >>> in asm vectors "
	$(ASM) $(ASMFLAGS) -o arm_start.o arm_start.s >> make.out

$(PROGRAM).o: $(PROGRAM).c Makefile
	@echo "GCC-ARM $< -o $@ >>> in program "
	$(CC) $(CFLAGS) -o $(PROGRAM).o $(PROGRAM).c  >> make.out

%.o: %.c 
	@echo "GCC-ARM $< -o $@ >>> in o -> c "
	$(CC) $(CFLAGS) -o $@ $< >> make.out

clean:
	@echo RM -rf targets and cruft
	@rm -rf $(OBJS) $(PROGRAM) $(PROGRAM).mem $(PROGRAM).map $(PROGRAM).bin *.o *.map *.bin *.mem make.out

clean_everything:
	@echo RM everything
	@rm -rf $(OBJS)	   \
		bfloat_mac         \
		bfloat_mac_perf    \
		factorial	   \
		hello              \
		mac                \
		mac_perf           \
		sha256             \
		sha256_perf        \
		*.mem              \
		*.map              \
		*.bin              \
		*.o                \
		*.map              \
		*.bin              \
		*.mem              \
		make.out

