#include <stdio.h>
#include <stdlib.h>

int main(int argument_count, char *argument_list[])
{
    char *r;
    char char_buf[100];
    char line[100];
    float value[10];
    FILE *input_image;
    FILE *output_image;
    int i;
    int n;
    int j;
    int p;
    int neg;
    int min;
    int max;
    float f;
    int count = 0;
    int values_per_word = 1;
    int width = 32/values_per_word;
    int fractional_bits = width/2;
    unsigned long outgoing_word[4];
    unsigned long t;
    float tf;
    unsigned long bits;

    if (argument_count != 4) {
        printf("usage: %s <par> <weights> <output_file> \n", argument_list[0]);
        return 1;
    }

    values_per_word = atoi(argument_list[1]);
    if ((values_per_word < 0) || (6 < values_per_word)) {
        fprintf(stderr, "that's not a good value for par (%d)\n", values_per_word);
        exit(2);
    }

    input_image = fopen(argument_list[2], "r");

    if (input_image==NULL) {
        fprintf(stderr, "unable to open file %s for reading \n", argument_list[1]);
        perror("oops");
        exit(2);
    }

    output_image = fopen(argument_list[3], "w");

    if (input_image==NULL) {
        fprintf(stderr, "unable to open file %s for writing \n", argument_list[2]);
        perror("oops");
        exit(2);
    }

    width = 32/values_per_word;
    fractional_bits = width/2;

    while (!feof(input_image)) {
        for (i=0; i<4; i++) {
            outgoing_word[i] = 0;
            for (p=0; p<values_per_word; p++) {
                max = 0;
                min = 0;
                r = fgets(char_buf, (size_t) sizeof(char_buf), input_image);
                if (r) {
                    count++;
                    t = strtoul(char_buf, NULL, 16);
                    tf = *(float *) (void *) &t;
printf("%f \n", tf);
                    if (tf > (1<<(width-fractional_bits))) max = 1;
                    if (tf < (-1 * (1<<(width-fractional_bits)))) min = 1;
                    value[p] = tf * ((float) (1 << fractional_bits));
                } else {
                    value[p] = 0.0;
                }
                bits = ((int) value[p]) & ((1l << width) - 1);
printf("%08x\n", bits);
                if (max) bits = ((1UL) << (width -1)) -1;
                if (min) bits = ((1UL) << (width -1));
                outgoing_word[i] |= (bits << (p * width));
printf("outgoing_word[%d]: %08x \n", i, outgoing_word[i]);
            }
        }
        fprintf(output_image, "%08x%08x%08x%08x\n", outgoing_word[3], outgoing_word[2], outgoing_word[1], outgoing_word[0]);
    }

    fclose(input_image);
    fclose(output_image);

    printf("processed %d values \n", count);
    return 0;
}


