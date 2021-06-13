

#ifdef HOST

#include <average_slave.hpp>

#else 

#include "average_slave.h"

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
    data_pipe_t values;
    sync_t         go;
    sync_t         done;
    count_t        count = n;
    volatile bool  sync_bit = true;
    int            i;
    data_t         answer;

    go.write(true);

    for (i=0; i<n; i++) values.write(array[i]);

    average_slave(count, values, answer, go, done);

    sync_bit = done.read();

    return answer;
}

#else // ARM

int average_hw(int array[], int n)
{
    int i;
    int answer;
/*
    int x;

    COUNT_REG = n;

    while (!GO_READY_REG);

    GO_REG = 1;

    for (i=0; i<n; i++) {
       while (!VALUES_READY_REG);
       VALUES_REG = array[i];
    }

    while (!DONE_VALID_REG);

    x = DONE_REG;

    answer = ANSWER_REG;
*/

    SET_COUNT(n);

    GO;

    for (i=0; i<n; i++) SET_VALUES(array[i]);

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
