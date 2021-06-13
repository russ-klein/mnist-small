solution file add ./testbench.cpp 
solution options set /Input/CompilerFlags {-I . -DSYNTHESIS -DSLAVE -DHOST -DA53 }

go analyze
go compile

solution library add nangate-45nm_beh -- -rtlsyntool DesignCompiler -vendor Nangate -technology 045nm
solution library add ccs_sample_mem
solution library add amba

go libraries

directive set /average_master/core -DESIGN_GOAL Latency
directive set -CLOCKS {clk {-CLOCK_PERIOD 5 -CLOCK_EDGE rising -CLOCK_HIGH_TIME 2.5 -CLOCK_OFFSET 0.000000 -CLOCK_UNCERTAINTY 0.0 -RESET_KIND async -RESET_SYNC_NAME rst -RESET_SYNC_ACTIVE high -RESET_ASYNC_NAME arst_n -RESET_ASYNC_ACTIVE low -ENABLE_NAME {} -ENABLE_ACTIVE high}}

directive set /average_master/memory:rsc -MAP_TO_MODULE {amba.ccs_axi4_master_core ADDR_WIDTH=44 ID_WIDTH=6 REGION_MAP_SIZE=4}

go assembly
go architect
go allocate
go extract

