solution file add ./testbench.cpp 
solution options set /Input/CompilerFlags {-I ../../../../python_mnist -DSYNTHESIS -DMASTER -DHOST -DWEIGHT_MEMORY -DFIXED_POINT -DPAR_IN=3 -DM3 }

go analyze
go compile

solution library add nangate-45nm_beh -- -rtlsyntool DesignCompiler -vendor Nangate -technology 045nm
solution library add ccs_sample_mem
solution library add ccs_ramifc_w_handshake -file ../../../ram_w_hs/ccs_ramifc_w_handshake.lib

go libraries

directive set /conv_par_in/core -DESIGN_GOAL Latency
directive set -CLOCKS {clk {-CLOCK_PERIOD 100 -CLOCK_EDGE rising -CLOCK_HIGH_TIME 50 -CLOCK_OFFSET 0.000000 -CLOCK_UNCERTAINTY 0.0 -RESET_KIND async -RESET_SYNC_NAME rst -RESET_SYNC_ACTIVE high -RESET_ASYNC_NAME arst_n -RESET_ASYNC_ACTIVE low -ENABLE_NAME {} -ENABLE_ACTIVE high}}

go assembly

directive set /conv_par_in/core/perform_convolution:shift_register:rsc -MAP_TO_MODULE {[Register]}

directive set /conv_par_in/core/main_convolve_loop -PIPELINE_INIT_INTERVAL 2

go architect

ignore_memory_precedences -from write_line:while:write_mem* -to write_line:while:if:read_mem*
ignore_memory_precedences -from write_line#2:while:write_mem* -to write_line#2:while:if:read_mem*
ignore_memory_precedences -from write_line#4:while:write_mem* -to write_line#4:while:if:read_mem*

go allocate
go extract

