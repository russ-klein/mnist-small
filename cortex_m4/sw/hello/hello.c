
#ifdef ARM
void console_out(char *s, ...);
#define printf console_out
#endif // ARM


static char a[64];
static short b[32];
static long c[16];


void memory_test_2()
{
    int i, j, k;

    printf("start: \n");

    for (i=0; i<10; i++) {
        *(((unsigned long *) 0x30000) + i) = i;
    }

    for (i=0; i<10; i++) {
        j = *(((unsigned long *) 0x30000) + i);
        printf("%d: %d \n",i, j);
    }

    for (i=0; i<10; i++) {
        *(((unsigned long *) 0x40000000) + i) = i;
    }

    for (i=0; i<10; i++) {
        j = *(((unsigned long *) 0x40000000) + i);
        printf("%d: %d \n",i, j);
    }

    for (i=0; i<10; i++) {
        *(((unsigned long *) 0xA0000000) + i) = i;
    }

    for (i=0; i<10; i++) {
        j = *(((unsigned long *) 0xA0000000) + i);
        printf("%d: %d \n",i, j);
    }
    printf("done: \n");
}

void memory_test()
{
    int i, j, k;

    printf("start \n");

    for (i=0; i<64; i++) {
       a[i] = i;
    }

    for (j=0; j<32; j++) {
       b[j] = j;
    }

    for (k=0; k<16; k++) {
       c[k] = k;
    }

    printf("writes done \n");

    for (i=0; i<64; i++) {
       if (a[i] != i) printf("expected: %d got: %d \n", i, a[i]);
    }

    for (j=0; j<32; j++) {
       if (b[j] != j) printf("expected: %d got: %d \n", j, b[j]);
    }

    for (k=0; k<16; k++) {
       if (c[k] != k) printf("expected: %d got: %d \n", k, c[k]);
    }

    printf("done \n");
   
}


int main()
{
    memory_test();
    memory_test_2();
    printf("Hello, world \n");
    return 0;
}
