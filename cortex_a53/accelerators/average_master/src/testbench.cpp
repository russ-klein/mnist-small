

#ifdef HOST

#include <average_master.hpp>

#else 

#include "average_master.h"

void console_out(char *s, ...);

#define printf console_out

#endif

#define N 10

int average_sw(int array[], int n)
{
    int i;
    int sum = 0;

    for (i=0; i<n; i++) sum += array[i];

    if (n > 0) return sum / n;
    else return 0;
}

#ifdef HOST

int average_hw(int array[], int n)
{
    offset_t       value_offset;
    sync_t         go;
    sync_t         done;
    count_t        count = n;
    int            written;
    count_t        mem_addr;
    data_t         value;
    volatile bool  sync_bit = true;
    int            i;
    data_t         answer;
    memory_line_t  memory[0x10000];
    memory_line_t  memory_line;

    value_offset = 0;
    written = 0;
    mem_addr = 0;

    while (written < n) {
        for (i=0; i<4; i++) {
            if (written < n) {
               value = array[written++];
            } else {
               value = 0;
            }
           memory_line.set_slc(i*32, (uint32) value);
        }
        memory[mem_addr++] = memory_line;
    }

    go.write(true);

    average_master(memory, count, value_offset, answer, go, done);

    sync_bit = done.read();

    return answer;
}

#else // ARM

int average_hw(int array[], int n)
{
    int i;
    int answer;
    int *memory = (int *) 0x40000000;

    for (i=0; i<n; i++) *(memory + i) = array[i];

    SET_COUNT(n);
    SET_VALUE_OFFSET(0);

    GO;

    WAIT_FOR_DONE;

    GET_ANSWER(answer);

    return answer;
}

#endif

int main()
{
    int       array[N];
    int       num = N;
    int       i;
    int       sw_ave;
    int       hw_ave;

    for (i=0; i<N; i++) array[i] = i;

    sw_ave = average_sw(array, N);
    hw_ave = average_hw(array, N);

    printf("sw_ave = %d hw_ave = %d \n", sw_ave, hw_ave);

    if (sw_ave == hw_ave) printf("Success! \n"); else printf("Failure! \n");
    
}
