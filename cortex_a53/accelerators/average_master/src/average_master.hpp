
#include <ac_int.h>
#include <ac_channel.h>
#include <ac_math/ac_div.h>

typedef ac_int<23, false>    data_t;
typedef ac_int<14, false>    offset_t;
typedef ac_int<9, false>     count_t;
typedef ac_channel<bool>     sync_t;

typedef ac_int<128, false>   memory_line_t;

#pragma hls_design top
void average_master(
    memory_line_t memory[0x10000],
    count_t       count,
    offset_t      value_offset,
    data_t       &answer,
    sync_t       &go,
    sync_t       &done)
{
    count_t       i;
    data_t        sum = 0;
    bool          sync_bit;
    data_t        q, r;
    offset_t      start;
    offset_t      end;
    offset_t      read_count;
    offset_t      mem_addr;
    memory_line_t memory_line;

    sync_bit = go.read();

    start       = value_offset;
    end         = value_offset + count;
    read_count  = 0;
    mem_addr    = value_offset >> 2;

    while (read_count<count) {
        memory_line = memory[mem_addr];
        for (i=0; i<4; i++) {
           if ((start <= ((mem_addr << 2) + i)) && (((mem_addr << 2) + i) < end)) {
              read_count++;
              sum += memory_line.slc<32>(i*32);
           }
        }
        mem_addr++;
    }

    ac_math::ac_div(sum,count,q,r);

    answer = q;

    done.write(sync_bit);
}
