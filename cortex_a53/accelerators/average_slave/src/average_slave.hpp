
#include <ac_int.h>
#include <ac_channel.h>
#include <ac_math/ac_div.h>

typedef ac_int<23, false>    data_t;
typedef ac_int<9, false>     count_t;
typedef ac_channel<data_t>   data_pipe_t;
typedef ac_channel<bool>     sync_t;

#pragma hls_design top
void average_slave(
    count_t       count,
    data_pipe_t  &values,
    data_t       &answer,
    sync_t       &go,
    sync_t       &done)
{
    count_t i;
    data_t sum = 0;
    bool sync_bit;
    data_t q, r;

    sync_bit = go.read();

    for (i=0; i<count; i++) {
       sum += values.read();
    }

    ac_math::ac_div(sum,count,q,r);

    answer = q;

    done.write(sync_bit);
}

