
solution file add ./testbench.cpp -exclude true
solution file add ./mac.cpp

go analyze
go compile

solution options set /ComponentLibs/SearchPath /calypto/technology_group/ajay/dc_flow/ce_dev/trunk/C_Proto_Setup/GF_12LP/catapult_libs -append
solution options set /ComponentLibs/TechLibSearchPath /calypto/c2proto_library_data/global_foundries/12LP/std_cell/12lp_hls_collaterals/12lp/invecas/STDLIB/7P5T/PDKV1.0_1.0/BASE/LVT/FDK/IN12LP_SC7P5T_84CPP_BASE_SSC14L_FDK_RELV00R50/model/timing/lib

#solution options set /ComponentLibs/TechLibSearchPath /calypto/c2proto_library_data/global_foundries/12LP/std_cell/12lp_hls_collaterals/12lp/invecas/STDLIB/7P5T/PDKV1.0_1.0/BASE/LVT/FDK/IN12LP_SC7P5T_84CPP_BASE_SSC14L_FDK_RELV00R50/model/timing/lib -append

solution library add IN12LP_SC7P5T_84CPP_BASE_SSC14L_SSPG_0P72V_M40C_or -- -rtlsyntool Oasys -rtlsyntool OasysRTL -vendor VENDOR -technology technology

#solution library add nangate-45nm_beh -- -rtlsyntool DesignCompiler -vendor Nangate -technology 045nm
#solution library add ccs_sample_mem

go libraries

directive set -CLOCKS {clk {-CLOCK_PERIOD 3.33 -CLOCK_EDGE rising -CLOCK_HIGH_TIME 1.665 -CLOCK_OFFSET 0.000000 -CLOCK_UNCERTAINTY 0.0 -RESET_KIND async -RESET_SYNC_NAME rst -RESET_SYNC_ACTIVE high -RESET_ASYNC_NAME arst_n -RESET_ASYNC_ACTIVE low -ENABLE_NAME {} -ENABLE_ACTIVE high}}

go assembly

directive set /mac/core -DESIGN_GOAL Latency

go extract
