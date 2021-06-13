solution file add ./testbench.cpp
solution file add ./sha256.cpp -args {-DHANDSHAKE}
solution file add ./sha256_defines.h

go compile

solution library add nangate-45nm_beh -- -rtlsyntool DesignCompiler -vendor Nangate -technology 045nm
solution library add ccs_sample_mem

go libraries

directive set -CLOCKS {clk {-CLOCK_PERIOD 10 -CLOCK_EDGE rising -CLOCK_HIGH_TIME 5 -CLOCK_OFFSET 0.000000 -CLOCK_UNCERTAINTY 0.0 -RESET_KIND async -RESET_SYNC_NAME rst -RESET_SYNC_ACTIVE high -RESET_ASYNC_NAME arst_n -RESET_ASYNC_ACTIVE low -ENABLE_NAME {} -ENABLE_ACTIVE high}}
directive set /sha256::run/core -DESIGN_GOAL Latency

go assembly

#directive set /sha256::run/core/this.transform:m:rsc -MAP_TO_MODULE {[Register]}
#directive set /sha256::run/core/if:if#2:hash_buffer:rsc -MAP_TO_MODULE {[Register]}
#directive set /sha256::run/core/this.transform#1:m:rsc -MAP_TO_MODULE {[Register]}
#directive set /sha256::run/core/this.transform#2:m:rsc -MAP_TO_MODULE {[Register]}
#directive set /sha256::run/core/ctx.data:rsc -MAP_TO_MODULE {[Register]}
#
#directive set /sha256::run/core/this.transform:for -UNROLL yes
#directive set /sha256::run/core/this.transform:for#1 -UNROLL yes
#directive set /sha256::run/core/this.transform:for#2 -UNROLL yes
#directive set /sha256::run/core/this.transform#1:for -UNROLL yes
#directive set /sha256::run/core/this.transform#1:for#1 -UNROLL yes
#directive set /sha256::run/core/this.transform#1:for#2 -UNROLL yes
#directive set /sha256::run/core/this.transform#2:for -UNROLL yes
#directive set /sha256::run/core/this.transform#2:for#1 -UNROLL yes
#directive set /sha256::run/core/this.transform#2:for#2 -UNROLL yes
#directive set /sha256::run/core/if:if#2:for -UNROLL yes
#
go allocate
go extract
