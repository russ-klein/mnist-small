
#====== Location Specific Settings ============

CATAPULT_HOME    ?= /wv/hlsb/CATAPULT/10.6/PRODUCTION/aol/Mgc_home
#CATAPULT_HOME    ?= tool/eda/apps/mentorCatapult/10.5/Mgc_home

#=============================================

FUNCTION         ?= mac
ACCEL_NAME       ?= cat_accel

SOURCES          = $(FUNCTION).cpp
SPEC_FILE        = $(FUNCTION).spec

ACCELERATOR      = ../hw/$(FUNCTION).v
BUS_IF           = ../hw/$(ACCEL_NAME).v
HEADER           = ../sw/$(FUNCTION).h

HOST_EXE         = $(FUNCTION)

CRUFT            = Catapult* catapult.log make.out design_checker_constraints.tcl design_checker_pre_build.tcl

CATAPULT         = $(CATAPULT_HOME)/bin/catapult
CATAPULT_FLAGS   = -shell -file directives.tcl

ifgen            = ../../../if_gen/if_gen

.PHONY: all run_host clean

all: $(ACCELERATOR) $(HEADER) $(BUS_IF) $(HOST_EXE)

run_host: $(HOST_EXE)
	@echo "Running host testbench"
	./$(HOST_EXE)

$(ACCELERATOR): $(SOURCES) directives.tcl
	@echo "catapult $(SOURCES) -o $(ACCELERATOR)"
	@rm -rf $(CRUFT)
	@$(CATAPULT) $(CATAPULT_FLAGS) >> make.out
	@cp Catapult/*/concat_rtl.v $@
	@rm -rf $(CRUFT)

$(HOST_EXE): $(SOURCES) testbench.cpp
	@echo "gcc $(SOURCES) -o $@"
	@g++ -o $@ -I $(CATAPULT_HOME)/shared/include -I . -std=c++11 testbench.cpp $(SOURCES) >> make.out

$(BUS_IF): $(SPEC_FILE) $(SOURCES)
	@echo ifgen $(SPEC_FILE) > $@
	@$(ifgen) $(FUNCTION) $(SPEC_FILE) $(ACCEL_NAME)
	mv $(ACCEL_NAME).v $(BUS_IF)
	mv $(FUNCTION).h $(HEADER)

$(HEADER): $(SPEC_FILE) $(SOURCES)
	@echo if_gen $(SPEC_FILE) > $@
	@$(ifgen) $(FUNCTION) $(SPEC_FILE) $(ACCEL_NAME)
	mv $(ACCEL_NAME).v $(BUS_IF)
	mv $(FUNCTION).h $(HEADER)

clean:
	rm -rf $(CRUFT) $(ACCELERATOR) $(HOST_EXE) $(BUS_IF) $(HEADER)

